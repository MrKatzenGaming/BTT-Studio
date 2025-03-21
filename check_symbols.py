import os

def parse_symbols(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    symbols = {}
    current_smo = None

    for line in lines:
        line = line.strip()
        if line.startswith('@smo:'):
            current_smo = line.split(':')[1]
            symbols[current_smo] = set()
        elif line and current_smo:
            symbols[current_smo].add(line)

    return symbols

def process_symbol_file(file_path):
    symbols = parse_symbols(file_path)

    smo_100_101_110_120 = symbols.get('100,101,110,120', set())
    smo_130 = symbols.get('130', set())

    result = smo_100_101_110_120 - smo_130

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
