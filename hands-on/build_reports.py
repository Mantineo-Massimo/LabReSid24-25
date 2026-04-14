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
            # Use batchmode to prevent stuck prompts, and increased timeout
            try:
                # Clean up auxiliary files first to ensure fresh Index generation
                subprocess.run(
                    ["latexmk", "-C"],
                    cwd=tex_dir,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL
                )
                
                subprocess.run(
                    ["latexmk", "-pdf", "-interaction=batchmode", f"Report_HO{i}.tex"],
                    cwd=tex_dir,
                    timeout=120,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL
                )
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

            # The compile should create ../Report_HO{i}.pdf because of .latexmkrc
            # but if it creates it in LaTex/ directory, we move it
            compiled_pdf = os.path.join(tex_dir, f"Report_HO{i}.pdf")
            target_pdf = os.path.join(ho_dir, f"Report_HO{i}.pdf")
            if os.path.exists(compiled_pdf):
                os.rename(compiled_pdf, target_pdf)
                print(f"Moved {compiled_pdf} to {target_pdf}")
