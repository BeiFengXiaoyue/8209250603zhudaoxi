import pdfplumber

pdf_path = r"C:\Users\BeiFengXiaoYue\Downloads\EduPlatform_master_代码扫描报告_20260717090902.pdf"
pdf = pdfplumber.open(pdf_path)

for i in range(260, 270):
    if i >= len(pdf.pages):
        break
    page = pdf.pages[i]
    text = page.extract_text()
    if text and ("G.FUN.02" in text or "参数名" in text):
        print(f"=== Page {i+1} ===")
        print(text[:2000])
        print()
