import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torchvision import datasets, transforms
from torch.utils.data import DataLoader, Subset
import ray
import numpy as np
from collections import OrderedDict

# 1. Simple MLP Model (from Assignment)
class SimpleMLP(nn.Module):
    def __init__(self):
        super(SimpleMLP, self).__init__()
        self.fc1 = nn.Linear(28*28, 256)
        self.fc2 = nn.Linear(256, 128)
        self.fc3 = nn.Linear(128, 10)

    def forward(self, x):
        x = x.view(-1, 28*28)
        x = F.relu(self.fc1(x))
        x = F.relu(self.fc2(x))
        x = self.fc3(x)
        return x

def get_weights(model):
    return {k: v.cpu().clone() for k, v in model.state_dict().items()}

def set_weights(model, weights):
    model.load_state_dict(OrderedDict(weights))

# 2. Federated Worker (Ray Actor)
@ray.remote
class FederatedWorker:
    def __init__(self, worker_id, train_dataset):
        self.worker_id = worker_id
        # Sharded training data for this worker
        self.train_loader = DataLoader(train_dataset, batch_size=64, shuffle=True)
        self.device = torch.device("cpu")
        self.model = SimpleMLP().to(self.device)

    def train(self, global_weights, epochs=1):
        set_weights(self.model, global_weights)
        optimizer = optim.Adam(self.model.parameters(), lr=0.001)
        criterion = nn.CrossEntropyLoss()
        
        self.model.train()
        for epoch in range(epochs):
            for data, target in self.train_loader:
                data, target = data.to(self.device), target.to(self.device)
                optimizer.zero_grad()
                output = self.model(data)
                loss = criterion(output, target)
                loss.backward()
                optimizer.step()
        
        # Return updated weights and number of examples processed
        return get_weights(self.model), len(self.train_loader.dataset)

# 3. Federated Aggregator
class FederatedAggregator:
    def __init__(self, num_workers=6):
        self.num_workers = num_workers
        self.device = torch.device("cpu")
        self.global_model = SimpleMLP().to(self.device)
        
        # Load and shard MNIST
        transform = transforms.Compose([
            transforms.ToTensor(),
            transforms.Normalize((0.1307,), (0.3081,))
        ])
        
        # Download and load training data
        full_train_dataset = datasets.MNIST(root='./data', train=True, download=True, transform=transform)
        
        # Load test data for evaluation
        self.test_loader = DataLoader(
            datasets.MNIST(root='./data', train=False, download=True, transform=transform),
            batch_size=1000, shuffle=False
        )
        
        # Split indices among 6 workers
        indices = np.arange(len(full_train_dataset))
        np.random.shuffle(indices)
        shards = np.array_split(indices, num_workers)
        
        # Initialize Workers as Ray Actors
        self.workers = [
            FederatedWorker.remote(i, Subset(full_train_dataset, shards[i]))
            for i in range(num_workers)
        ]

    def aggregate(self, reports):
        """Perform Federated Averaging (FedAvg)"""
        weights_list = [r[0] for r in reports]
        num_examples_list = [r[1] for r in reports]
        total_examples = sum(num_examples_list)
        
        new_weights = OrderedDict()
        for key in weights_list[0].keys():
            # Weighted average of layers
            new_weights[key] = sum(
                [weights[key] * (num_examples / total_examples) 
                 for weights, num_examples in zip(weights_list, num_examples_list)]
            )
        
        set_weights(self.global_model, new_weights)

    def evaluate(self, round_num):
        """Evaluate the global model on the central test set"""
        self.global_model.eval()
        correct = 0
        total = 0
        with torch.no_grad():
            for data, target in self.test_loader:
                data, target = data.to(self.device), target.to(self.device)
                output = self.global_model(data)
                _, pred = torch.max(output, 1)
                correct += pred.eq(target).sum().item()
                total += target.size(0)
        
        acc = correct / total
        print(f"Round {round_num} - Global Test Accuracy: {acc*100:.2f}%")
        return acc

    def run_federated_learning(self, num_rounds=3):
        print(f">>> Initializing Federated Learning with {self.num_workers} workers...")
        for r in range(1, num_rounds + 1):
            print(f">>> Round {r} starting...")
            global_weights = get_weights(self.global_model)
            
            # 1. Dispatch global weights to workers for local training
            training_ids = [worker.train.remote(global_weights) for worker in self.workers]
            
            # 2. Wait for all workers to finish
            print(f"Waiting for {self.num_workers} workers to report...")
            reports = ray.get(training_ids)
            
            # 3. Aggregate local models into global model (FedAvg)
            print(f"Aggregating weights for round {r}...")
            self.aggregate(reports)
            
            # 4. Global Evaluation
            acc = self.evaluate(r)
            print(f"Round {r} completed. Global Accuracy: {acc*100:.2f}%")

if __name__ == "__main__":
    # Ensure Ray is initialized
    print("Inizializzazione Ray...")
    if not ray.is_initialized():
        ray.init(ignore_reinit_error=True, logging_level="info")
    
    print("Dataset loading and sharding...")
    aggregator = FederatedAggregator(num_workers=6)
    aggregator.run_federated_learning(num_rounds=3) 
    
    print("Training finished. Shutting down Ray.")
    ray.shutdown()
