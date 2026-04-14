import ray
import os
import re
from collections import Counter

# Initialize Ray
ray.init(ignore_reinit_error=True)

@ray.remote
def map_task(text_chunk):
    # Preprocess and count words in the chunk
    words = re.findall(r'\w+', text_chunk.lower())
    return Counter(words)

def map_reduce_word_count(file_path, num_mappers=4):
    if not os.path.exists(file_path):
        print(f"Error: {file_path} not found.")
        return

    with open(file_path, 'r', encoding='utf-8') as f:
        text = f.read()

    # Split text into chunks
    chunk_size = len(text) // num_mappers
    chunks = [text[i:i + chunk_size] for i in range(0, len(text), chunk_size)]

    # Start Map tasks
    result_ids = [map_task.remote(chunk) for chunk in chunks]
    
    # Collect and Reduce results
    final_counts = Counter()
    for partial_count_id in result_ids:
        final_counts.update(ray.get(partial_count_id))

    return final_counts

if __name__ == "__main__":
    input_file = "lotr.txt"
    output_file = "lotr_report.txt"
    
    print(f"Starting distributed MapReduce on {input_file}...")
    results = map_reduce_word_count(input_file)
    
    if results:
        # Sort by frequency descending
        sorted_results = results.most_common()
        
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write("=== Lord of the Rings Word Frequency Report ===\n")
            f.write(f"Total Unique Words: {len(sorted_results)}\n")
            f.write("-" * 47 + "\n")
            f.write(f"{'WORD':<30} | {'COUNT':<10}\n")
            f.write("-" * 47 + "\n")
            for word, count in sorted_results:
                f.write(f"{word:<30} | {count:<10}\n")
        
        print(f"Report generated: {output_file}")
        print(f"Top 10 words:")
        for word, count in sorted_results[:10]:
            print(f"  {word}: {count}")

    ray.shutdown()
