import sys
try:
    import pdfplumber
    print("pdfplumber imported", flush=True)
except Exception as e:
    print(f"Import error: {e}", flush=True)
    sys.exit(1)

try:
    pdf = pdfplumber.open(r"C:\Users\BeiFengXiaoYue\Downloads\EduPlatform_master_代码扫描报告_20260717090902.pdf")
    print(f"Total pages: {len(pdf.pages)}", flush=True)
    for i, page in enumerate(pdf.pages):
        text = page.extract_text()
        if text:
            print(f"=== Page {i+1} ===", flush=True)
            print(text[:3000], flush=True)
            print(flush=True)
        else:
            print(f"=== Page {i+1} === (no text)", flush=True)
except Exception as e:
    print(f"Error: {e}", flush=True)
    sys.exit(1)
