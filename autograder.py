import subprocess
import signal
import sys

def pretty_print(i):
    print()
    print('#' * 30)
    print("#{}#".format(' ' * 28))
    if i < 10:
        print('#   Evaluating TestCase ${}   #'.format(i))
    # guess we don't have over 100 testcases, hopefully...
    else:
        print('#   Evaluating TestCase ${}  #'.format(i))
    print("#{}#".format(' ' * 28))
    print('#' * 30)
    print()

class TestCase:

    def __init__(self, outName, isPIE, score, name, retVal, output, *src) -> None:
        self.outName = outName
        self.binName = outName[:-2]
        self.isPIE = isPIE
        self.src = src
        self.score = score
        self.claimedScore = 0
        self.name = name
        self.retVal = retVal
        self.output = output
        self.init_command()
    
    def init_command(self):
        self.command = ['./build/ics-linker']
        self.command.extend(self.src)
        self.command.append('-o')
        self.command.append(self.outName)
        if self.isPIE == False:
            self.command.append('-no-pie')
        self.debugCommand = ['gdb', '--args']
        self.debugCommand.extend(self.command)
    
    def eval(self):
        print("Test name:", self.name)
        print("Command:", " ".join(self.command))
        
        try:
            p = subprocess.run(self.command, check=True, capture_output=True, encoding='utf-8')
            print("Last words from stdout:", p.stdout, sep='\n')
            print("Last words from stderr:", p.stderr, sep='\n')
        except subprocess.CalledProcessError as e:
            if e.returncode == -signal.SIGSEGV:
                print("SIGSEGV received in your linker. Maybe you want to debug it with gdb.")
            elif e.returncode == -signal.SIGABRT:
                print("SIGABRT received in your linker. Exit as expected.")
                if self.output != None:
                    if self.output == e.stderr:
                        print("Test passed!")
                        self.claimedScore += self.score
                    else:
                        print("Unexpected output: ", e.stderr)
                else:
                    print("Last words from stdout: ", e.stdout)
                    print("Last words from stderr: ", e.stderr)
            else:
                print("Oops, your linker does not return correctly.")
                print("Last words from stdout:", e.stdout)
                print("Last words from stderr:", e.stderr)
                print("Bad return value:", e.returncode)
            # The linker is no good, return
            return

        try:
            p = subprocess.run(self.binName, check=True, capture_output=True, encoding='utf-8')
            print("Last words from stdout:", p.stdout, sep='\n')
            print("Last words from stderr:", p.stderr, sep='\n')
        except subprocess.CalledProcessError as e:
            if e.returncode == self.retVal:
                self.claimedScore += self.score
                print("Test passed!")
            elif e.returncode == -signal.SIGSEGV:
                print("SIGSEGV received in linked program. Maybe you want to debug it with gdb.")
                print("Or you might want to start with objdump -d ", self.outName)
            elif e.returncode == -signal.SIGABRT:
                print("SIGABRT received in your linker. Exit as expected (but this should not happen).")
                print("Last words from stdout: ", e.stdout)
                print("Last words from stderr: ", e.stderr)
            else:
                print("Oops, your program does not return correctly.")
                print("It's probably because references are bound to wrong symbols")

if __name__ == '__main__':
    claimedScore = 0

    test0 = TestCase('testcases/test0.o', True, 25, 'Simple Single Relocation', 2, None, 'testcases/glbvar.o')
    pretty_print(0)
    test0.eval()
    claimedScore += test0.claimedScore

    test1 = TestCase('testcases/test1.o', False, 25, 'Direct Relocation', 3, None, 'testcases/sum.o')
    pretty_print(1)
    test1.eval()
    claimedScore += test1.claimedScore

    test2 = TestCase("testcases/test2.o", True, 25, 'Undefined Reference', None, 
                     "undefined reference for symbol foo\n", 'testcases/extcall.o')
    pretty_print(2)
    test2.eval()
    claimedScore += test2.claimedScore

    test3 = TestCase("testcases/test3.o", True, 25, 'Multiple Definition', None,
                     "multiple definition for symbol a\n", 'testcases/multidef1.o', 'testcases/multidef2.o')
    pretty_print(3)
    test3.eval()
    claimedScore += test3.claimedScore
    
    print(f"\nTotal score: {claimedScore}/100")