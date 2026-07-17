import pdfplumber, sys, os

pdf_path = r"C:\Users\BeiFengXiaoYue\Downloads\EduPlatform_master_代码扫描报告_20260717090902.pdf"
pdf = pdfplumber.open(pdf_path)

# Find pages with detailed issue locations
for i in range(5, min(40, len(pdf.pages))):
    page = pdf.pages[i]
    text = page.extract_text()
    if text and len(text.strip()) > 50:
        print(f"=== Page {i+1} ===")
        print(text[:1500])
        print()
