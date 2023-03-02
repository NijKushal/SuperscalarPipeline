#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <list>
#include <iostream>
#include <algorithm>
#include "sim_proc.h"

using namespace std;

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim 256 32 4 gcc_trace.txt
    argc = 5
    argv[0] = "sim"
    argv[1] = "256"
    argv[2] = "32"
    ... and so on
*/
int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    proc_params params;       // look at sim_bp.h header file for the the definition of struct proc_params
    int op_type, dest, src1, src2;  // Variables are read from trace file
    unsigned long int pc; // Variable holds the pc read from input file
    
    if (argc != 5)
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.rob_size     = strtoul(argv[1], NULL, 10);
    params.iq_size      = strtoul(argv[2], NULL, 10);
    params.width        = strtoul(argv[3], NULL, 10);
    trace_file          = argv[4];
    num_instr           = 0;
    num_cycles          = 0;


    ROB ROB_table;
    RMT RMT_table;
    Issue_Queue issueQueue;
    vector<Instr_Bundle> decode_bundle;
    vector<Instr_Bundle> rename_bundle;
    vector<Instr_Bundle> regRead_bundle;
    vector<Instr_Bundle> dispatch_bundle;
    vector<Instr_Bundle> execute_list;
    vector<Instr_Bundle> writeback_bundle;
    vector<Instr_Bundle> retire_bundle;



    ROB_table.head = ROB_table.tail = 3;
    ROB_table.rob_size = params.rob_size;
    ROB_ENTRY temp;
    for(int i = 0; i < params.rob_size; i++)
    {
        ROB_table.table.push_back(temp);
    }

    issueQueue.iq_size = params.iq_size;

    for(int i = 0; i < 67; i++)
    {
        RMT_table.reg_list[i].rob_tag = 0;
        RMT_table.reg_list[i].valid = false;
    }


    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }

    bool EOF_flag = false;
    bool pipeline_empty;
    int seq=0;
    do 
    {

        retire(retire_bundle, regRead_bundle, params.width, seq, ROB_table, RMT_table);
        write_Back(ROB_table, writeback_bundle, retire_bundle);
        execute(regRead_bundle, dispatch_bundle, issueQueue, execute_list, writeback_bundle);
        issue(issueQueue, execute_list, params.width);
        dispatch(dispatch_bundle, issueQueue);
        reg_Read(regRead_bundle, dispatch_bundle, ROB_table);
        rename(rename_bundle, regRead_bundle, ROB_table, RMT_table);
        decode(decode_bundle, rename_bundle);
        fetch(params.width, decode_bundle, FP, EOF_flag);
        pipeline_empty = decode_bundle.empty() && rename_bundle.empty() && regRead_bundle.empty()
                && dispatch_bundle.empty() && issueQueue.IQ.empty() && ROB_table.head == ROB_table.tail && execute_list.empty()
                && writeback_bundle.empty();
    } while (Advance_Cycle(EOF_flag, pipeline_empty));


    cout<<"# === Simulator Command =========\n";
    cout<<"# ./sim"<<params.rob_size<<" "<<params.iq_size<<" "<<params.width<<" "<<trace_file<<"\n"; 
    cout<<"# === Processor Configuration ===\n";
    cout<<"# ROB_SIZE = "<<params.rob_size<<"\n";
    cout<<"# IQ_SIZE  = "<<params.iq_size<<"\n";
    cout<<"# WIDTH    = "<<params.width<<"\n";
    cout<<"# === Simulation Results ========\n";
    cout<<"# Dynamic Instruction Count    = "<< num_instr<<"\n";
    cout<<"# Cycles                       = %d\n"<< num_cycles<<"\n";
    cout<<"# Instructions Per Cycle (IPC) = "<< (float(num_instr)/float(num_cycles))<<"\n";

    return 0;
}