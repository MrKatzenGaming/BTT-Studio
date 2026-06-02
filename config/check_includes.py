#!/usr/bin/env python3
"""
Check include patterns in C++ source files.
Enforces absolute paths from project root for your code.
Excludes specified library paths from checks.
"""

import argparse
import re
import sys
from pathlib import Path
from typing import List, Tuple

# Patterns to check
QUOTED_INCLUDE = re.compile(r'#include\s+"([^"]+)"')
ANGLE_INCLUDE = re.compile(r'#include\s+<([^>]+)>')

# Paths to exclude from checking
EXCLUDED_PATHS = [
    "lib/OdysseyHeaders",
    "lib/imgui",
    "sys/",
]

# For your code, enforces absolute paths from project root or include-relative paths
# OdysseyHeaders libraries (can be included directly by namespace)
LIBRARY_NAMESPACES = re.compile(r'^(al|sead|nn|vapours|agl|eui|game|nvn|custom|hk)/')


def is_library_namespace(include_path: str) -> bool:
    """Return whether an include path refers to a library namespace."""
    return LIBRARY_NAMESPACES.match(include_path) is not None


def fix_includes_in_file(file_path: Path, project_root: Path) -> int:
    """Rewrite includable library namespace quotes to angle brackets."""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()
    except Exception:
        return 0

    changed = 0
    for index, line in enumerate(lines):
        match = QUOTED_INCLUDE.search(line)
        if not match:
            continue

        include_path = match.group(1)
        if not is_library_namespace(include_path):
            continue

        fixed_line = line.replace(f'"{include_path}"', f'<{include_path}>', 1)
        if fixed_line != line:
            lines[index] = fixed_line
            changed += 1

    if changed:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.writelines(lines)

    return changed

def file_exists_at_any_location(include_path: str, project_root: Path) -> bool:
    """Check if the included file exists at any valid location."""
    
    # Library namespaces: check in lib/custom and lib/OdysseyHeaders
    if is_library_namespace(include_path):
        # Check lib/custom first (for overrides)
        custom_path = project_root / "lib" / "custom" / include_path
        if custom_path.exists():
            return True
        # Check lib/OdysseyHeaders
        odyssey_path = project_root / "lib" / "OdysseyHeaders" / include_path
        if odyssey_path.exists():
            return True
        # Check lib/OdysseyHeaders/NintendoSDK (for nn, nvn, vapours)
        nintendo_sdk_path = project_root / "lib" / "OdysseyHeaders" / "NintendoSDK" / include_path
        if nintendo_sdk_path.exists():
            return True
        return False
    
    # Absolute paths from project root (src/ or include/)
    if include_path.startswith("src/"):
        full_path = project_root / include_path
        return full_path.exists()
    
    if include_path.startswith("include/"):
        full_path = project_root / include_path
        return full_path.exists()
    
    if include_path == "imgui.h" or include_path == "imgui_internal.h":
        return True
    
    # Relative paths: assume they're relative to include/
    if not include_path.startswith(("src/", "include/", "/", "..")):
        include_rel_path = project_root / "include" / include_path
        return include_rel_path.exists()
    
    return False

def is_valid_include(include_path: str, project_root: Path, source_file: Path) -> bool:
    """Check if an include path is valid and points to an existing file."""
    
    # Check basic pattern validity
    if include_path.startswith(("src/", "include/", "/")):
        # Absolute paths - make sure they're not escaping
        if ".." in include_path:
            return False
    elif not include_path.startswith(("..")):
        # Relative or library namespace paths
        if ".." in include_path:
            return False
    else:
        return False
    
    # Check if file actually exists
    return file_exists_at_any_location(include_path, project_root)

def should_check_file(file_path: Path, project_root: Path) -> bool:
    """Check if file should be validated."""
    rel_path = file_path.relative_to(project_root)
    
    # Skip excluded paths
    for excluded in EXCLUDED_PATHS:
        if str(rel_path).startswith(excluded):
            return False
    
    # Only check source files in src/ and include/ and lib/custom
    if str(rel_path).startswith(("src/", "include/", "lib/custom")):
        return True
    
    return False

def check_includes_in_file(file_path: Path, project_root: Path) -> List[Tuple[int, str, str]]:
    """
    Check includes in a file.
    Returns list of (line_number, include_statement, error_message)
    """
    errors = []
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()
    except Exception as e:
        return [(0, "", f"Error reading file: {e}")]
    
    rel_file_path = file_path.relative_to(project_root)
    
    for line_num, line in enumerate(lines, 1):
        # Check quoted includes
        match = QUOTED_INCLUDE.search(line)
        if match:
            include_path = match.group(1)
            if is_library_namespace(include_path):
                errors.append((
                    line_num,
                    line.strip(),
                    f"Library include '{include_path}' must use angle brackets, not quotes"
                ))
                continue
            # Check if file exists
            if not file_exists_at_any_location(include_path, project_root):
                errors.append((
                    line_num,
                    line.strip(),
                    f"Include file not found: '{include_path}'"
                ))
            # Check pattern validity
            elif not is_valid_include(include_path, project_root, file_path):
                errors.append((
                    line_num,
                    line.strip(),
                    f"Invalid include path pattern: '{include_path}'. Use one of:\n"
                    f"    - Absolute from project root: 'src/module/file.h' or 'include/subdir/file.h'\n"
                    f"    - Include-relative: 'subdir/file.h' (resolves to include/subdir/file.h)\n"
                    f"    - Library namespaces: 'al/file.h', 'sead/file.h', 'nn/file.h', etc."
                ))
        
        # Check angle includes (these are typically for external libs, which is fine)
        match = ANGLE_INCLUDE.search(line)
        if match:
            # Angle includes are allowed for external dependencies
            pass
    
    return errors

def main():
    parser = argparse.ArgumentParser(description="Check and optionally fix include patterns.")
    parser.add_argument("files", nargs="*", help="Source files to check")
    parser.add_argument("--fix", action="store_true", help="Automatically fix library includes to use angle brackets")
    args = parser.parse_args()

    if not args.files:
        print("Usage: check_includes.py [--fix] <source_files...>")
        sys.exit(0)
    
    project_root = Path(__file__).parent.parent
    all_errors = []
    total_fixes = 0
    
    for file_arg in args.files:
        file_path = Path(file_arg)
        if not file_path.is_absolute():
            file_path = project_root / file_path
        
        if not should_check_file(file_path, project_root):
            continue

        if args.fix:
            total_fixes += fix_includes_in_file(file_path, project_root)
        
        errors = check_includes_in_file(file_path, project_root)
        for line_num, include_stmt, error_msg in errors:
            print(f"{file_path}:{line_num}: {error_msg}")
            print(f"  {include_stmt}")
            all_errors.append((file_path, line_num, error_msg))
    
    if all_errors:
        print(f"\nFound {len(all_errors)} include pattern violations")
        sys.exit(1)
    else:
        if args.fix and total_fixes:
            print(f"Fixed {total_fixes} include(s)")
        print("All include patterns are valid")
        sys.exit(0)

if __name__ == "__main__":
    main()
