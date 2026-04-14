import os
import subprocess
import glob

hands_on_dir = "/home/massimo/Scrivania/LabReSid24-25/hands-on"

for i in range(1, 20):
    ho_dir = os.path.join(hands_on_dir, f"ho{i}")
    tex_dir = os.path.join(ho_dir, "LaTex")
    if os.path.exists(tex_dir):
        # Rename Relazione_HO{i}.tex to Report_HO{i}.tex
        old_tex = os.path.join(tex_dir, f"Relazione_HO{i}.tex")
        new_tex = os.path.join(tex_dir, f"Report_HO{i}.tex")
        if os.path.exists(old_tex):
            print(f"Renaming {old_tex} to {new_tex}")
            os.rename(old_tex, new_tex)
        
        # Compile Report_HO{i}.tex
        if os.path.exists(new_tex):
            print(f"Compiling {new_tex}...")
            try:
                # Clean up auxiliary files first
                subprocess.run(
                    ["latexmk", "-C"],
                    cwd=tex_dir,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL
                )
                
                # Use nonstopmode to see errors in logs but not hang
                result = subprocess.run(
                    ["latexmk", "-pdf", "-interaction=nonstopmode", f"Report_HO{i}.tex"],
                    cwd=tex_dir,
                    timeout=120,
                    capture_output=True,
                )
                
                output = result.stdout.decode('utf-8', errors='replace')
                
                if result.returncode != 0:
                    print(f"FAILED to compile {new_tex} (Exit Code {result.returncode})")
                    # Check log for errors if it exists
                    log_file = os.path.join(tex_dir, f"Report_HO{i}.log")
                    if os.path.exists(log_file):
                        with open(log_file, 'r', encoding='utf-8', errors='replace') as f:
                            log_content = f.read()
                            errors = [line for line in log_content.splitlines() if line.startswith('!')]
                            for err in errors[:5]:
                                print(f"  {err}")

            except subprocess.TimeoutExpired:
                print(f"Timeout compiling {new_tex}")
            except Exception as e:
                print(f"Error compiling {new_tex}: {e}")
            
            # Verify .toc file
            toc_file = os.path.join(tex_dir, f"Report_HO{i}.toc")
            if os.path.exists(toc_file):
                size = os.path.getsize(toc_file)
                if size > 0:
                    print(f"Index generated for HO{i} ({size} bytes)")
                else:
                    print(f"WARNING: Index empty for HO{i}")
            else:
                print(f"WARNING: Index NOT generated for HO{i}")

            # With .latexmkrc, PDF is created in hoX/ (parent of LaTex/)
            # We check both locations to be safe
            target_pdf = os.path.join(ho_dir, f"Report_HO{i}.pdf")
            local_pdf = os.path.join(tex_dir, f"Report_HO{i}.pdf")
            
            if os.path.exists(local_pdf):
                if os.path.exists(target_pdf):
                    os.remove(target_pdf)
                os.rename(local_pdf, target_pdf)
                print(f"Moved {local_pdf} to {target_pdf}")
            elif os.path.exists(target_pdf):
                print(f"PDF correctly placed in {target_pdf}")
            else:
                print(f"ERROR: PDF not found for HO{i}")
