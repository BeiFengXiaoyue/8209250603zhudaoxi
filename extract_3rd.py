import pdfplumber, sys

pdf_path = r"C:\Users\BeiFengXiaoYue\Downloads\EduPlatform_master_代码扫描报告_20260717093400.pdf"
pdf = pdfplumber.open(pdf_path)
print(f"Total pages: {len(pdf.pages)}", flush=True)
for i in range(min(10, len(pdf.pages))):
    page = pdf.pages[i]
    text = page.extract_text()
    if text:
        print(f"=== Page {i+1} ===", flush=True)
        print(text[:2500], flush=True)
