import pdfplumber

pdf_path = r"C:\Users\BeiFengXiaoYue\Downloads\EduPlatform_master_代码扫描报告_20260717090902.pdf"
pdf = pdfplumber.open(pdf_path)

# Find the G.VAR.03, G.INC.04-CPP, G.RES.06-CPP, G.FUN.02-CPP issues
search_terms = ["G.VAR.03", "G.INC.04-CPP", "G.RES.06-CPP", "G.FUN.02-CPP", 
                "G.ERR.04", "G.ERR.07", "G.CLS.03-CPP", "G.CTL.03", "G.LOG.02",
                "G.INC.11-CPP", "G.NAM.03-CPP", "G.CMT.05-CPP",
                "G.INC.06-CPP", "G.NAM.02-CPP"]

for i in range(len(pdf.pages)):
    page = pdf.pages[i]
    text = page.extract_text()
    if text:
        for term in search_terms:
            if term in text:
                print(f"=== Page {i+1} ===")
                print(text[:2000])
                print()
                break
