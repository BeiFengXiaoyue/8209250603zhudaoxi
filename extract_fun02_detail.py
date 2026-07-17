import pdfplumber

pdf_path = r"C:\Users\BeiFengXiaoYue\Downloads\EduPlatform_master_代码扫描报告_20260717092422.pdf"
pdf = pdfplumber.open(pdf_path)

# G.FUN.02-CPP: pages around 247-250
for i in range(245, 260):
    if i >= len(pdf.pages):
        break
    page = pdf.pages[i]
    text = page.extract_text()
    if text:
        print(f"=== Page {i+1} ===")
        print(text[:2000])
        print()
