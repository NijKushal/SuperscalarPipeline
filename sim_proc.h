#ifndef SIM_PROC_H
#define SIM_PROC_H

#include <vector>
#include <list>
#include <iostream>
#include <algorithm>

using namespace std;

typedef unsigned int u_int;
typedef unsigned long u_ll;

typedef struct proc_params{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
}proc_params;

    int num_cycles, num_instr;  //global


class Instr_Bundle
{
    public:
    u_int op_type;
    u_int dst;
    u_int src1;
    u_int src2;
    u_int latency;
    u_int age_in_iss;
    u_int src1_non_rob;
    u_int src2_non_rob;
    u_int dst_non_rob;
    u_ll pc;
    bool rs1_rob;
    bool rs1_rdy;
    bool rs2_rob;
    bool rs2_rdy;
    u_int FE_begin;
    u_int FE_cycles;
    u_int DE_begin;
    u_int DE_cycles;
    u_int RN_begin;
    u_int RN_cycles;
    u_int RR_begin;
    u_int RR_cycles;
    u_int DI_begin;
    u_int DI_cycles;
    u_int IS_begin;
    u_int IS_cycles;
    u_int EX_begin;
    u_int EX_cycles;
    u_int WB_begin;
    u_int WB_cycles;
    u_int RT_begin;
    u_int RT_cycles;
};

class Issue_Queue
{
public:
    unsigned long iq_size;
    vector<Instr_Bundle> IQ;
};

class RMT
{
private:
    class RMT_Entry
    {
    public:
        bool valid;
        int rob_tag;
    };
public:
    RMT_Entry reg_list[67];
};

class ROB_ENTRY{
public:
    unsigned int pc;
    int dest;
    bool rdy;
    ROB_ENTRY(): pc(0), dest(0), rdy(false){}
    void clr()
    {
        pc = 0; 
        dest = 0; 
        rdy = false;
    }
    bool empty()
    {
        return pc == 0 && dest == 0 && !rdy;
    }
};

class ROB
{
public:
    u_int head, tail;
    u_ll rob_size;
    vector<ROB_ENTRY> table;
    unsigned long space_available()
    {
        unsigned long avail_entries;
        if(tail < head)
            avail_entries = head - tail;
        else if(head < tail)
            avail_entries = rob_size - (tail - head);
        else
        {
            if(tail < (rob_size - 1))
            {
                if(table[tail+1].empty())
                {
                    avail_entries = rob_size;
                }    
                else
                {
                    avail_entries = 0;
                }    
            }
            else
            {
                if(table[tail-1].empty())
                {
                    avail_entries = rob_size;
                }    
                else
                {
                    avail_entries = 0;
                }    
            }
        }
        return avail_entries;
    };
};

struct comparator
{
    inline bool operator() (const Instr_Bundle &temp1, const Instr_Bundle &temp2)
    {
        return (temp1.age_in_iss < temp2.age_in_iss);
    }
};
// Put additional data structures here as per your requirement
   

void retire(vector<Instr_Bundle> &retire_bundle, vector<Instr_Bundle> &regRead_bundle, u_ll width, int &seq, ROB &ROB_table, RMT &RMT_table)
{
    int num_retired = 0;
    while (num_retired < width) 
    {
        if ((ROB_table.tail == ROB_table.head) && (ROB_table.head != ROB_table.rob_size - 1)) 
        {
            if (ROB_table.table[ROB_table.head + 1].pc == 0) 
            {
                return;
            }
        }
        if (ROB_table.table[ROB_table.head].rdy) 
        {
            for (int i = 0; i < regRead_bundle.size(); i++) 
            {
                if (ROB_table.head == regRead_bundle.at(i).src1) 
                {
                    regRead_bundle.at(i).rs1_rdy = true;
                }
                if (ROB_table.head == regRead_bundle.at(i).src2) 
                {
                    regRead_bundle.at(i).rs2_rdy = true;
                }
            }
            for (int j = 0; j < 67; j++) 
            {
                if (ROB_table.head == RMT_table.reg_list[j].rob_tag && RMT_table.reg_list[j].valid) 
                {
                    RMT_table.reg_list[j].valid = false;
                    RMT_table.reg_list[j].rob_tag = 0;
                }
            }
            for (int k = 0; k < retire_bundle.size(); k++) 
            {
                if (ROB_table.table[ROB_table.head].pc == retire_bundle.at(k).pc) 
                {
                    retire_bundle.at(k).RT_cycles = (num_cycles + 1) - retire_bundle.at(k).RT_begin;


                    cout<<seq<<" fu{"<<retire_bundle.at(k).op_type<<"}";
                    cout<<"src{"<<retire_bundle.at(k).src1_non_rob<<","<<retire_bundle.at(k).src2_non_rob<<"}";
                    cout<<"dst{"<<retire_bundle.at(k).dst_non_rob<<"}";
                    cout<<"FE{"<<retire_bundle.at(k).FE_begin<<","<<retire_bundle.at(k).FE_cycles<<"}";
                    cout<<"DE{"<<retire_bundle.at(k).DE_begin<<","<<retire_bundle.at(k).DE_cycles<<"}";
                    cout<<"RN{"<<retire_bundle.at(k).RN_begin<<","<<retire_bundle.at(k).RN_cycles<<"}";
                    cout<<"RR{"<<retire_bundle.at(k).RR_begin<<","<<retire_bundle.at(k).RR_cycles<<"}";
                    cout<<"DI{"<<retire_bundle.at(k).DI_begin<<","<<retire_bundle.at(k).DI_cycles<<"}";
                    cout<<"IS{"<<retire_bundle.at(k).IS_begin<<","<<retire_bundle.at(k).IS_cycles<<"}";
                    cout<<"EX{"<<retire_bundle.at(k).EX_begin<<","<<retire_bundle.at(k).EX_cycles<<"}";
                    cout<<"WB{"<<retire_bundle.at(k).WB_begin<<","<<retire_bundle.at(k).WB_cycles<<"}";
                    cout<<"RT{"<<retire_bundle.at(k).RT_begin<<","<<retire_bundle.at(k).RT_cycles<<"}"<<endl;
                    seq++;

                    retire_bundle.erase(retire_bundle.begin() + k);
                    break;
                }
            }

            ROB_table.table[ROB_table.head].clr();

            if (ROB_table.head != (ROB_table.rob_size - 1)) 
            {
                ROB_table.head++;
            } else 
            {
                ROB_table.head = 0;
            }

            num_retired++;
        }
        else
        {
            return;
        }
    }
};


void write_Back(ROB &ROB_table, vector<Instr_Bundle> &writeback_bundle, vector<Instr_Bundle> &retire_bundle)
{
    if(!writeback_bundle.empty())
    {
        for(int i = 0; i < writeback_bundle.size(); i++)
        {
            writeback_bundle.at(i).RT_begin = num_cycles + 1;
            writeback_bundle.at(i).WB_cycles = writeback_bundle.at(i).RT_begin - writeback_bundle.at(i).WB_begin;
            ROB_table.table[writeback_bundle.at(i).dst].rdy = true;
            retire_bundle.push_back(writeback_bundle.at(i));
        }
        writeback_bundle.clear();
    }
};



void execute(vector<Instr_Bundle> &regRead_bundle, vector<Instr_Bundle> &dispatch_bundle, Issue_Queue &issueQueue, vector<Instr_Bundle> &execute_list, vector<Instr_Bundle> &writeback_bundle)
{
    if(!execute_list.empty()) 
    {
        for (int j = 0; j < execute_list.size(); j++) 
        {
            execute_list.at(j).latency--;
        }
        int temp = 1;
        while (temp != 0) 
        {
            temp = 0;
            for (int i = 0; i < execute_list.size(); i++) 
            {
                if (execute_list.at(i).latency == 0) 
                {
                    execute_list.at(i).WB_begin = num_cycles + 1;
                    execute_list.at(i).EX_cycles = execute_list.at(i).WB_begin - execute_list.at(i).EX_begin;
                    writeback_bundle.push_back(execute_list.at(i));
                    //IQ
                    for (int j = 0; j < issueQueue.IQ.size(); j++) {
                        if (issueQueue.IQ.at(j).src1 == execute_list.at(i).dst) 
                        {
                            issueQueue.IQ.at(j).rs1_rdy = true;
                        }
                        if (issueQueue.IQ.at(j).src2 == execute_list.at(i).dst) 
                        {
                            issueQueue.IQ.at(j).rs2_rdy = true;
                        }
                    }
                    //DI
                    for (int k = 0; k < dispatch_bundle.size(); k++) {
                        if (dispatch_bundle.at(k).src1 == execute_list.at(i).dst) 
                        {
                            dispatch_bundle.at(k).rs1_rdy = true;
                        }
                        if (dispatch_bundle.at(k).src2 == execute_list.at(i).dst) 
                        {
                            dispatch_bundle.at(k).rs2_rdy = true;
                        }
                    }
                    //RR
                    for (int l = 0; l < regRead_bundle.size(); l++) {
                        if (regRead_bundle.at(l).src1 == execute_list.at(i).dst) 
                        {
                            regRead_bundle.at(l).rs1_rdy = true;
                        }
                        if (regRead_bundle.at(l).src2 == execute_list.at(i).dst) 
                        {
                            regRead_bundle.at(l).rs2_rdy = true;
                        }
                    }
                    execute_list.erase(execute_list.begin() + i);
                    temp++;
                    break;
                }
            }
        }
    }
};


void issue(Issue_Queue &issueQueue, vector<Instr_Bundle> &execute_list, unsigned long width)
{
    if(!issueQueue.IQ.empty())
    {
        sort(issueQueue.IQ.begin(), issueQueue.IQ.end(), comparator());
        int control = 1;
        int issued = 0;
        while(control != 0) 
        {
            control = 0;
            for (int i = 0; i < issueQueue.IQ.size(); i++) 
            {
                if (issueQueue.IQ.at(i).rs1_rdy && issueQueue.IQ.at(i).rs2_rdy) 
                {
                    issueQueue.IQ.at(i).EX_begin = num_cycles + 1;
                    issueQueue.IQ.at(i).IS_cycles = issueQueue.IQ.at(i).EX_begin - issueQueue.IQ.at(i).IS_begin;
                    execute_list.push_back(issueQueue.IQ.at(i));
                    issueQueue.IQ.erase(issueQueue.IQ.begin() + i);
                    issued++;
                    control++;
                    break;
                }
            }
            if (issued == width) 
            {
                 break;
            }
        }
    }
};


void dispatch(vector<Instr_Bundle> &dispatch_bundle, Issue_Queue &issueQueue)
{
    if(!dispatch_bundle.empty() && ((issueQueue.iq_size - issueQueue.IQ.size()) >= dispatch_bundle.size()))
    {
        for(int i = 0; i < dispatch_bundle.size(); i++)
        {
            dispatch_bundle.at(i).IS_begin = num_cycles + 1;
            dispatch_bundle.at(i).DI_cycles = dispatch_bundle.at(i).IS_begin - dispatch_bundle.at(i).DI_begin;
            issueQueue.IQ.push_back(dispatch_bundle.at(i));
        }
        dispatch_bundle.clear();
    }
};



void reg_Read(vector<Instr_Bundle> &regRead_bundle, vector<Instr_Bundle> dispatch_bundle, ROB &ROB_table)
{
    if(dispatch_bundle.empty() && !regRead_bundle.empty())
    {
        for(int i = 0; i < regRead_bundle.size(); i++)
        {
            if(regRead_bundle.at(i).rs1_rob) 
            {
                if (ROB_table.table[regRead_bundle.at(i).src1].rdy)
                {
                    regRead_bundle.at(i).rs1_rdy=true;
                }    
            }
            else
            {    
                regRead_bundle.at(i).rs1_rdy=true;
            }
            if(regRead_bundle.at(i).rs2_rob) 
            {
                if (ROB_table.table[regRead_bundle.at(i).src2].rdy)
                {
                    regRead_bundle.at(i).rs2_rdy=true;
                }   
            }    
            else
            {
                regRead_bundle.at(i).rs2_rdy=true;  
            }
            
            regRead_bundle.at(i).DI_begin = num_cycles + 1;
            regRead_bundle.at(i).RR_cycles = regRead_bundle.at(i).DI_begin - regRead_bundle.at(i).RR_begin;                               
        }
        regRead_bundle.swap(dispatch_bundle);
        regRead_bundle.clear();
    }
};


void rename(vector<Instr_Bundle> &rename_bundle, vector<Instr_Bundle> &regRead_bundle, ROB &ROB_table, RMT &RMT_table)
{
    if(regRead_bundle.empty() && !rename_bundle.empty())
    {
        if(ROB_table.space_available()<rename_bundle.size())
        {
            return;
        }
        else
        {
            for(int i=0; i<rename_bundle.size(); i++)
            {
                ROB_table.table[ROB_table.tail].dest = rename_bundle.at(i).dst;
                ROB_table.table[ROB_table.tail].pc = rename_bundle.at(i).pc;
                ROB_table.table[ROB_table.tail].rdy = false;
                if(rename_bundle.at(i).src1 != -1)
                {
                    if(RMT_table.reg_list[rename_bundle.at(i).src1].valid)
                    {
                        rename_bundle.at(i).src1 = RMT_table.reg_list[rename_bundle.at(i).src1].rob_tag;
                        rename_bundle.at(i).rs1_rob = true;
                    }
                }
                if(rename_bundle.at(i).src2 != -1)
                {
                    if(RMT_table.reg_list[rename_bundle.at(i).src2].valid)
                    {
                        rename_bundle.at(i).src2 = RMT_table.reg_list[rename_bundle.at(i).src2].rob_tag;
                        rename_bundle.at(i).rs2_rob = true;
                    }
                }
                if(rename_bundle.at(i).dst != -1)
                {
                    RMT_table.reg_list[rename_bundle.at(i).dst].valid = true;
                    RMT_table.reg_list[rename_bundle.at(i).dst].rob_tag = ROB_table.tail;
                }
                rename_bundle.at(i).dst = ROB_table.tail;
                if(ROB_table.tail != (ROB_table.rob_size - 1))
                    ROB_table.tail++;
                else
                    ROB_table.tail = 0;
                rename_bundle.at(i).RR_begin = num_cycles + 1;
                rename_bundle.at(i).RN_cycles = rename_bundle.at(i).RR_begin - rename_bundle.at(i).RN_begin;
            }
            rename_bundle.swap(regRead_bundle);
            rename_bundle.clear();
        }
    }
};

void decode(vector<Instr_Bundle> &decode_bundle, vector<Instr_Bundle> &rename_bundle)
{
    if(!decode_bundle.empty() && rename_bundle.empty())
    {

        for(int i = 0; i < decode_bundle.size(); i++)
        {
            decode_bundle.at(i).RN_begin = num_cycles++;
            decode_bundle.at(i).DE_cycles = decode_bundle.at(i).RN_begin - decode_bundle.at(i).DE_begin;           
        }
        decode_bundle.swap(rename_bundle);
        decode_bundle.clear();
    }
};


void fetch(unsigned long int width, vector<Instr_Bundle> &decode_bundle, FILE *FP, bool &EOF_flag)
{
    int op_type, dest, src1, src2; 
    u_int pc;          
    if(decode_bundle.empty()) 
    {
        for(int i = 0; i<width; i++)
        {
            if (fscanf(FP, "%x %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF) 
            {
                num_instr+=1;
                Instr_Bundle entry_instr;
                entry_instr.pc = pc;
                entry_instr.op_type = op_type;
                entry_instr.dst = dest;
                entry_instr.dst_non_rob = dest;
                entry_instr.src1 = src1;
                entry_instr.src1_non_rob = src1;
                entry_instr.src2 = src2;
                entry_instr.src2_non_rob = src2;                
                entry_instr.rs1_rdy = false;
                entry_instr.rs2_rdy = false;
                entry_instr.rs1_rob = false;
                entry_instr.rs2_rob = false;
                entry_instr.age_in_iss = 0;
                if(op_type == 0)
                    entry_instr.latency = 1;
                else if(op_type == 1)
                    entry_instr.latency = 2;
                else
                    entry_instr.latency = 5;

                entry_instr.FE_cycles = 1;                
                entry_instr.FE_begin = num_cycles;
                entry_instr.DE_begin = num_cycles+1;
     
                decode_bundle.push_back(entry_instr); 
                EOF_flag = false;
            }
            else 
            {
                EOF_flag = true;
            }
        }
    }
}

bool Advance_Cycle(bool EOF_Flag, bool pipeline_empty)
{
    num_cycles++;
    if(!(EOF_Flag && pipeline_empty))
    return true;
    else
    return false;
}

#endif