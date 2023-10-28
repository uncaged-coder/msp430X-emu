importPackage(Packages.com.ti.debug.engine.scripting);
importPackage(Packages.com.ti.ccstudio.scripting.environment);
importPackage(Packages.java.lang);

// Setup parameters
var STACK_START_ADDR = 0x30E0; // Set this to the start address of your stack
var STACK_SIZE = 0x300; // Size of the stack (in bytes)
var NB_INSTRUCTIONS = 10000;

var ccsInstallDir = "/../../opt/ti/ccs1240/";
var fwDir = "/../../zdata/progs/work/myopowers/icu_fw/";
var logFile ="/../../tmp/log.xml";
var logFileStylesheet = ccsInstallDir + "/ccs_base/scripting/examples/DebugServerExamples/DefaultStylesheet.xsl";
//var deviceCCXMLFile = fwDir + "/targetConfigs/MSP430F2618.ccxml";
var deviceCCXMLFile = "/zdata/progs/work/myopowers/icu-emu/tools/board.ccxml";
var fwFullPath = fwDir + "/Debug/ICU_FW.out";

// Initialize arrays to store the previous state of registers and stack
var prevRegValues = {};
var prevStackValues = {};

function dumpStack(startAddr, size) {
    var stackContent = [];
    for (var i = size - 2; i >= 0; i-=2) {
        stackContent.push(sv.memory.readWord(0, startAddr - i));
    }
    return stackContent;
}

function monitorChanges() {
    var changedRegs = {};
    var changedStackOffsets = [];

    // Check registers for changes
    for (var reg in prevRegValues) {
        var currVal = sv.memory.readRegister(reg);
        if (currVal != prevRegValues[reg]) {
            changedRegs[reg] = {
                oldValue: prevRegValues[reg],
                newValue: currVal
            };
            prevRegValues[reg] = currVal; // Update the previous value
        }
    }

    // Check stack for changes
    var currStackValues = dumpStack(STACK_START_ADDR, STACK_SIZE);
    for (var i = 0; i < STACK_SIZE / 2; i++) {
        if (currStackValues[i] != prevStackValues[i]) {
            changedStackOffsets.push({
                offset: i * 2,
                oldValue: prevStackValues[i],
                newValue: currStackValues[i]
            });
            prevStackValues[i] = currStackValues[i]; // Update the previous value
        }
    }

    return {
        regs: changedRegs,
        stack: changedStackOffsets
    };
}

function logToFile(message) {
    //fileOutput.push(message); // Temporarily store messages in an array
    print(message);
}

// Create Environment and open a debug session
var env = ScriptingEnvironment.instance();
env.traceBegin(logFile, logFileStylesheet);
env.traceSetConsoleLevel(TraceLevel.ALL);
env.traceSetFileLevel(TraceLevel.ALL);

var myServer = env.getServer("DebugServer.1");
print(myServer);
myServer.setConfig(deviceCCXMLFile);

//var sv = myServer.openSession("MSP430F2618");
var sv = myServer.openSession("*", "*");

sv.target.connect();

// Load a program
sv.memory.loadProgram(fwFullPath);

sv.breakpoint.add("main");

// Start the debbugger
sv.target.asmStep.into();
//sv.target.run();

// Initialize the previous state
for (var i = 0; i <= 15; i++) {
    var reg = "R" + i;
    if (i == 0)
        reg = "PC";
    else if (i == 1)
        reg = "SP";
    else if (i == 2)
        reg = "SR";
    prevRegValues[reg] = sv.memory.readRegister(reg);
}
prevStackValues = dumpStack(STACK_START_ADDR, STACK_SIZE);

// Dump current registers value
for (var reg in prevRegValues) {
    var currVal = sv.memory.readRegister(reg);
    logToFile("init reg " + reg + " = " + currVal.toString(16));
}

// The main loop
for (var countInstr = 0; countInstr <= NB_INSTRUCTIONS; countInstr++) {
    var pc = sv.memory.readRegister("PC");

    // Execute the instruction
    sv.target.asmStep.into();

    // Monitor changes
    var changes = monitorChanges();

    // Log the instruction, registers, and stack to a file
    var rawInstruction = sv.memory.readWord(0, pc).toString(16)
    logToFile("Instruction " + countInstr + " " + pc.toString(16) + ":" + rawInstruction);// +  ": " + mnemonic);
    //logToFile("Src Mode: " + srcMode + ", Dest Mode: " + destMode);
    for (var reg in changes.regs) {
        logToFile("Changed Reg: " + reg + ", Old: " + changes["regs"][reg]["oldValue"].toString(16) + ", New: " + changes["regs"][reg]["newValue"].toString(16));
    }
    for (var i = 0; i < changes.stack.length; i++) {
        var change = changes.stack[i];
        logToFile("Changed Stack: " + change["offset"] + ", Old: " + change["oldValue"].toString(16) + ", New: " + change["newValue"].toString(16));
    }
}

logToFile("Script done!");

//print(fileOutput.join('\n'));

myServer.stop();
env.traceEnd();
