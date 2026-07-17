"""Fix remaining missing braces in playerwidget.cpp"""
import re

path = r"C:\Users\BeiFengXiaoYue\Desktop\Qt\now\inTheEnd\client\video\playerwidget.cpp"
with open(path, 'r', encoding='utf-8') as f:
    lines = f.readlines()

# Patterns where single-line if/for/while needs braces
# Match: "    if (condition) statement;"  (no braces)
# But skip ones that already have braces or are inside strings
fixes = 0
i = 0
while i < len(lines):
    line = lines[i]
    stripped = line.rstrip()
    
    # Check for patterns like: "    if (xxx) return;" or "    if (xxx) y = 10;"
    # where the line has no opening brace and doesn't end with a brace
    if not stripped.endswith('{') and not stripped.endswith('}'):
        # Match: if/for/while (condition) single_statement;
        m = re.match(r'^(\s*)(if|for|while)\s*\(.*\)\s+(\S.*);\s*$', stripped)
        if m and '//' not in stripped.split(';')[0]:
            indent = m.group(1)
            keyword = m.group(2)
            cond_end = stripped.index(')') + 1
            condition = stripped[len(indent):cond_end]
            statement = m.group(3)
            
            # Replace the line with braces
            lines[i] = f"{indent}{condition} {{\n"
            lines.insert(i+1, f"{indent}    {statement};\n")
            lines.insert(i+2, f"{indent}}}\n")
            fixes += 1
            i += 2  # Skip the two inserted lines
    i += 1

# Also fix multi-line: if (cond)\n    stmt;
i = 0
while i < len(lines) - 1:
    line = lines[i]
    next_line = lines[i+1]
    stripped = line.rstrip()
    next_stripped = next_line.rstrip()
    
    # Check: "    if (cond)" followed by "        stmt;" (no braces around)
    m1 = re.match(r'^(\s*)(if|for|while)\s*\(.*\)\s*$', stripped)
    m2 = re.match(r'^\1\s{4}(\S.*;\s*)$', next_stripped) if m1 else None
    
    if m1 and not stripped.endswith('{'):
        indent = m1.group(1)
        keyword = m1.group(2)
        
        # Check if next line is just a statement without braces
        if next_stripped and not next_stripped.startswith('{') and not next_stripped.startswith('}') and not next_stripped.startswith('//'):
            # Also check the line after that isn't an else/catch
            lines[i] = f"{stripped} {{\n"
            lines[i+1] = f"{indent}    {next_stripped}\n"
            # Find where to close - look for the next line at same indent level
            for j in range(i+2, len(lines)):
                if re.match(r'^\s*\}\s*$', lines[j]) or re.match(r'^' + re.escape(indent) + r'\S', lines[j]):
                    if not lines[j].strip().startswith('}') and not lines[j].strip().startswith('else'):
                        lines.insert(j, f"{indent}}}\n")
                        break
            fixes += 1
            i += 1
    i += 1

with open(path, 'w', encoding='utf-8') as f:
    f.writelines(lines)

print(f"Fixed {fixes} missing brace issues")
