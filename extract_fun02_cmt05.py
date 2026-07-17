import pdfplumber

pdf_path = r"C:\Users\BeiFengXiaoYue\Downloads\EduPlatform_master_代码扫描报告_20260717092422.pdf"
pdf = pdfplumber.open(pdf_path)

for i in range(8, len(pdf.pages)):
    page = pdf.pages[i]
    text = page.extract_text()
    if text and ("G.FUN.02-CPP" in text or "G.CMT.05-CPP" in text):
        print(f"=== Page {i+1} ===")
        print(text[:2000])
        print()
