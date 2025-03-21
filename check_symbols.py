import os

def parse_symbols(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    symbols_100 = []
    symbols_130 = []
    current_smo = None

    for line in lines:
        line = line.strip()
        if line.startswith('@smo:'):
            current_smo = line.split(':')[1]
        elif line.startswith("@sdk"):
            return symbols_100, symbols_130
        elif line and current_smo == '100,101,110,120':
            symbols_100.append(line)
        elif line and current_smo == '130':
            symbols_130.append(line.split(" ")[0])

    return symbols_100, symbols_130

def process_symbol_file(file_path):
    if "custom" in file_path:
        return
    symbols = parse_symbols(file_path)
    
    smo_100_101_110_120 = symbols[0]
    smo_130 = symbols[1]
    
    result = smo_100_101_110_120

    result = [symbol for symbol in result if symbol not in smo_130]


    if not result:
        return
    # print(f"Symbols in {file_path} defined for smo:100,101,110,120 but not smo:130:")
    print(f"{file_path}")
    for symbol in result:
        print(symbol)
    print()

def main():
    directory = '/home/kassu/btt-mod/syms'
    for root, _, files in os.walk(directory):
        for file_name in files:
            if file_name.endswith('.sym'):
                file_path = os.path.join(root, file_name)
                process_symbol_file(file_path)

if __name__ == "__main__":
    main()
