import subprocess
import itertools
import random

def generate_random_bool_function(num_vars):
    var_names = [chr(ord('A') + i) for i in range(num_vars)]
    combinations = list(itertools.product([0, 1], repeat=num_vars))

    sop_terms = []
    truth_table = []

    for combo in combinations:
        output = random.randint(0, 1)
        truth_table.append((combo, output))
        if output == 1:
            term_parts = []
            for var, val in zip(var_names, combo):
                term_parts.append(var if val == 1 else f"!{var}")
            sop_terms.append('*'.join(term_parts))
    
    with open("Testing\\boolFuncs.txt", "w") as boolFuncFile, open("Testing\\expectedOutput.txt", "w") as expOutFile:
        boolFuncFile.write(f"{testing}\n")
        boolFuncFile.write(f"{printAns}\n")

        if sop_terms:
            bool_func = '+'.join(sop_terms)
        else:
            bool_func = "0"  # Function always outputs 0
        
        boolFuncFile.write(f"{bool_func}\n")
        
        for combo, output in truth_table:
            input_str = ''.join(map(str, combo))
            boolFuncFile.write(f"{input_str}\n")
            expOutFile.write(f"{output}\n")
    
    print("INPUT GENERATION FINISHED!")

def runExe(exe_path, input_txt, output_txt):
    # Open the input file to read values
    with open(input_txt, 'r') as input_file:
        input_data = input_file.read()
    
    # Open the output file to store the output
    with open(output_txt, 'w') as output_file:
        # Run the executable
        process = subprocess.Popen(
            exe_path, 
            stdin=subprocess.PIPE, 
            stdout=subprocess.PIPE, 
            stderr=subprocess.PIPE,
            text=True
        )
        
        # Pass input to the executable and capture output
        stdout, stderr = process.communicate(input=input_data)

        # Write output to file
        output_file.write(stdout)
        
        # If there's any error output, also log it
        if stderr:
            output_file.write("\nERROR OUTPUT:\n" + stderr)


if __name__ == "__main__":
    generateNewBoolFunc = input("Generate new bool function (1/0): ")

    if(generateNewBoolFunc == "1"):
        testing = input("Enable testing (1/0): ")
        printAns = input("Enable correctness test (1/0): ")
        n = int(input("Enter number of variables: "))
        generate_random_bool_function(n)

    exe_path = "BDD.exe"
    input_txt = "Testing\\boolFuncs.txt"
    output_txt = "Testing\\output.txt"
    runExe(exe_path, input_txt, output_txt)
    print("TESTS FINISHED!")