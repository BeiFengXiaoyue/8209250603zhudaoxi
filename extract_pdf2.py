import sys, os

try:
    import pdfplumber
except Exception as e:
    print(f"Import error: {e}")
    sys.exit(1)

pdf_path = r"C:\Users\BeiFengXiaoYue\Downloads\EduPlatform_master_代码扫描报告_20260717090902.pdf"
if not os.path.exists(pdf_path):
    print(f"File not found: {pdf_path}")
    sys.exit(1)

print(f"File size: {os.path.getsize(pdf_path)} bytes")
sys.stdout.flush()

try:
    pdf = pdfplumber.open(pdf_path)
    print(f"Pages: {len(pdf.pages)}")
    sys.stdout.flush()
    
    for i in range(min(5, len(pdf.pages))):
        page = pdf.pages[i]
        text = page.extract_text()
        if text:
            print(f"=== Page {i+1} ({len(text)} chars) ===")
            print(text[:2000])
            sys.stdout.flush()
        else:
            print(f"=== Page {i+1} (no text) ===")
            sys.stdout.flush()
except Exception as e:
    import traceback
    print(f"Error: {e}")
    traceback.print_exc()
    sys.exit(1)
