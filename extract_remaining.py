import pdfplumber

pdf_path = r"C:\Users\BeiFengXiaoYue\Downloads\EduPlatform_master_代码扫描报告_20260717092422.pdf"
pdf = pdfplumber.open(pdf_path)

# Collect all pages with G.FMT.11 or G.FUN.02 or G.CMT.05 info
for i in range(8, min(100, len(pdf.pages))):
    page = pdf.pages[i]
    text = page.extract_text()
    if text:
        # Show pages with specific rule references
        if any(x in text for x in ["G.FMT.11-CPP", "G.FUN.02-CPP", "G.CMT.05-CPP", "问题", "client/"]):
            print(f"=== Page {i+1} ===")
            print(text[:1500])
            print()
