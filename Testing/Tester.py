import subprocess
import time

def run_exe_with_io(exe_path, input_txt, output_txt):
    global execution_time

    # Open the input file to read values
    with open(input_txt, 'r') as input_file:
        input_data = input_file.read()
    
    # Start the timer
    start_time = time.perf_counter()
    
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
        
        # Stop the timer
        end_time = time.perf_counter()
        execution_time = end_time - start_time

        # Write output to file
        output_file.write(stdout)
        
        # If there's any error output, also log it
        if stderr:
            output_file.write("\nERROR OUTPUT:\n" + stderr)

# Example usage
exe_path = "BDD.exe"
input_txt = "Testing\\boolFuncs.txt"
output_txt = "Testing\\output.txt"
times_txt = "Testing\\times.txt"

# testRepeats = int(input("Test repeats: "))

timesFile = open(times_txt, 'w')

# for i in range(testRepeats):
run_exe_with_io(exe_path, input_txt, output_txt)
timesFile.write(f"{execution_time}\n")
# print(f"progress = {i+1}/{testRepeats}")

timesFile.close()
print("TESTS FINISHED!")