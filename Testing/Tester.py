import subprocess
import time

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


exe_path = "BDD.exe"
input_txt = "Testing\\boolFuncs.txt"
output_txt = "Testing\\output.txt"

runExe(exe_path, input_txt, output_txt)

print("TESTS FINISHED!")