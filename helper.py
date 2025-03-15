import subprocess
import os
import sys
import stat
import shutil

def run_test(test_no, commands, expected_output, description, skipPass = False):
    print(f"Test{test_no}: {description}", end='\t')
    with subprocess.Popen(
        ["./main.exe"],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    ) as proc:
        stdout, _ = proc.communicate(input=commands)
        if stdout.strip() != expected_output.strip():
            print("FAIL")
            print("Expected:")
            print(expected_output.strip())
            print("\nGot:")
            print(stdout.strip())
            print("\n")
        elif not skipPass:
            print("PASS")

existing_file_text = "This is a test file.\nIt has multiple lines.\n"             
def setup_files():
    with open("existing.txt", "w") as f:
        f.write(existing_file_text)

def run_tests():
    setup_files()

    # 1. Open an existing file
    run_test(1, "open existing.txt r\nexit\n", "File opened successfully.\n", "Open existing file")

    # 2. Open and close an existing file
    run_test(2, "open existing.txt r\nclose\nexit\n", "File opened successfully.\nFile closed successfully.\n", "Open and close existing file")

    # 3. Open a non-existing file
    run_test(3, "open non_existing.txt r\nexit\n", "Failed to open file.\n", "Open non-existing file")

    # 4. Read an already existing file
    run_test(4, "open existing.txt r\nread 10\nclose\nexit\n", "File opened successfully.\nRead: This is a \nFile closed successfully.\n", "Read from existing file")

    # 5. Write a new file
    file_content = "Hello"
    run_test(5, f"open newfile.txt w\nwrite {file_content}\nclose\nexit\n", "File opened successfully.\nWrite successful.\nFile closed successfully.\n", "Write a new file", True)
    try:
        with open("newfile.txt", "r") as file:
            content = file.read()
            if content != file_content:
                print("FAIL")
            else:
                print("PASS")
    except FileNotFoundError:
        print("FAIL - File was not created")

    # 6. Overwrite an existing file
    file_content = "Overwritten!"
    run_test(6, f"open existing.txt w\nwrite {file_content}\nclose\nexit\n", "File opened successfully.\nWrite successful.\nFile closed successfully.\n", "Overwrite an existing file", True)
    with open("existing.txt", "r") as file:
        content = file.read()
        if content != file_content:
            print("FAIL")
        else:
            print("PASS")

    # 7. Append to an existing file
    setup_files()
    file_content = "Appended text"
    run_test(7, f"open existing.txt a\nwrite {file_content}\nclose\nexit\n", "File opened successfully.\nWrite successful.\nFile closed successfully.\n", "Append to an existing file", True)
    with open("existing.txt", "r") as file:
        content = file.read()
        if content != f"{existing_file_text}{file_content}":
            print("FAIL")
            print(content)
            print(f"{existing_file_text}{file_content}")
        else:
            print("PASS")

    # 8. Read from the middle of an existing file
    run_test(8, "open existing.txt r\nseek 10 0\nread 5\nclose\nexit\n", "File opened successfully.\nSeek successful.\nRead: test \nFile closed successfully.\n", "Read from the middle of an existing file")

    # 9. Write to the middle of an existing file
    run_test(9, "open existing.txt r+\nseek 5 0\nwrite MIDDLE\nseek 5 0\nread 10\nclose\nexit\n", "File opened successfully.\nSeek successful.\nWrite successful.\nSeek successful.\nRead: MIDDLEest \nFile closed successfully.\n", "Write to the middle of an existing file")

def clean():
    os.remove("existing.txt")
    os.remove("newfile.txt")

def check():
    run_tests()
    clean()

def compile():
    folder = './build'
    rmtree(folder)

    os.chdir(folder)
    os.system('cmake ..')
    os.system('cmake --build .')
    shutil.copyfile('bin/Debug/main.exe', '../main.exe')

def rmtree(top):
    for root, dirs, files in os.walk(top, topdown=False):
        for name in files:
            filename = os.path.join(root, name)
            os.chmod(filename, stat.S_IWUSR)
            os.remove(filename)
        for name in dirs:
            os.chmod(os.path.join(root, name), stat.S_IWUSR)
            os.rmdir(os.path.join(root, name))

def print_usage():
    print("Usage:")
    print(f"{sys.argv[0]} --compile # will compile your solution using cmake")
    print(f"{sys.argv[0]} --check # will check your executable" )

if __name__ == "__main__":
    if len(sys.argv) < 2:
        check()

    if True:
        check()
    elif sys.argv[1] == "--check":
        check()
    else:
        print_usage()
    