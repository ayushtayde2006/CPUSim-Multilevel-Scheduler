#include <iostream>
#include <queue>
#include <vector>
#include <string>
#include <fstream>
using namespace std;

// Structure to represent a Task
struct Task {
    string id;        // Task name like T1, T2
    int burst;        // Total CPU time needed
    int remaining;    // Remaining time left
    vector<string> memBlocks; // Memory blocks it needs
};
vector<Task> readTasks(string filename) {
    vector<Task> tasks;
    ifstream file(filename);
    string word;
    
    while (file >> word) {
        if (word == "TASK") {
            Task t;
            file >> t.id;           // reads T1, T2 etc
            file >> word;           // reads "BURST"
            file >> t.burst;        // reads burst number
            t.remaining = t.burst;  // remaining = burst initially
            file >> word;           // reads "MEM"
            
            // Read all memory blocks until next TASK or end
            while (file >> word && word != "TASK") {
                t.memBlocks.push_back(word); // adds M1, M2 etc
            }
            tasks.push_back(t);
            
            // If we read next "TASK" word, put it back
            if (word == "TASK") {
                file.seekg(-4, ios::cur);
            }
        }
    }
    return tasks;
}


// Cache levels
queue<string> L1, L2, L3;
int L1_SIZE = 3, L2_SIZE = 5, L3_SIZE = 8;
int ramAccess = 0;

// Latencies
int L1_LAT = 4, L2_LAT = 12, L3_LAT = 40, RAM_LAT = 200;

bool searchAndPromote(string block, int &cycles) {
    // Check L1
    queue<string> temp = L1;
    while (!temp.empty()) {
        if (temp.front() == block) {
            cout << "    " << block << " -> HIT in L1 (" 
                 << L1_LAT << " cycles)" << endl;
            cycles += L1_LAT;
            return true;
        }
        temp.pop();
    }

    // Check L2
    temp = L2;
    queue<string> newL2;
    bool foundInL2 = false;
    while (!temp.empty()) {
        if (temp.front() == block) {
            foundInL2 = true;
        } else {
            newL2.push(temp.front());
        }
        temp.pop();
    }
    if (foundInL2) {
        cout << "    " << block << " -> MISS L1 -> HIT in L2 (" 
             << L2_LAT << " cycles)" << endl;
        cycles += L2_LAT;
        // Promote to L1
        if (L1.size() >= L1_SIZE) {
            // Move evicted L1 block to L2
            string evicted = L1.front();
            L1.pop();
            newL2.push(evicted);
            cout << "    Evicting " << evicted 
                 << " from L1 -> moved to L2" << endl;
        }
        L1.push(block);
        // Update L2 without the promoted block
        if (newL2.size() >= L2_SIZE) {
            string evicted = newL2.front();
            newL2.pop();
            cout << "    Evicting " << evicted << " from L2" << endl;
        }
        L2 = newL2;
        return true;
    }

    // Check L3
    temp = L3;
    queue<string> newL3;
    bool foundInL3 = false;
    while (!temp.empty()) {
        if (temp.front() == block) {
            foundInL3 = true;
        } else {
            newL3.push(temp.front());
        }
        temp.pop();
    }
    if (foundInL3) {
        cout << "    " << block << " -> MISS L1 -> MISS L2 -> HIT in L3 (" 
             << L3_LAT << " cycles)" << endl;
        cycles += L3_LAT;
        // Promote to L1, cascade evictions
        if (L1.size() >= L1_SIZE) {
            string evictedL1 = L1.front();
            L1.pop();
            cout << "    Evicting " << evictedL1 
                 << " from L1 -> moved to L2" << endl;
            if (L2.size() >= L2_SIZE) {
                string evictedL2 = L2.front();
                L2.pop();
                cout << "    Evicting " << evictedL2 
                     << " from L2 -> moved to L3" << endl;
                newL3.push(evictedL2);
            }
            L2.push(evictedL1);
        }
        L1.push(block);
        // Update L3 without promoted block
        if (newL3.size() >= L3_SIZE) {
            string evicted = newL3.front();
            newL3.pop();
            cout << "    Evicting " << evicted << " from L3" << endl;
        }
        L3 = newL3;
        return true;
    }

    // Not found anywhere - fetch from RAM
    cout << "    " << block 
         << " -> MISS L1 -> MISS L2 -> MISS L3 -> Fetching from RAM (" 
         << RAM_LAT << " cycles)" << endl;
    cycles += RAM_LAT;
    ramAccess++;

    // Cascade promotion L1 -> L2 -> L3
    if (L1.size() >= L1_SIZE) {
        string evictedL1 = L1.front();
        L1.pop();
        cout << "    Evicting " << evictedL1 
             << " from L1 -> moved to L2" << endl;
        if (L2.size() >= L2_SIZE) {
            string evictedL2 = L2.front();
            L2.pop();
            cout << "    Evicting " << evictedL2 
                 << " from L2 -> moved to L3" << endl;
            if (L3.size() >= L3_SIZE) {
                string evictedL3 = L3.front();
                L3.pop();
                cout << "    Evicting " << evictedL3 
                     << " from L3 (dropped)" << endl;
            }
            L3.push(evictedL2);
        }
        L2.push(evictedL1);
    }
    L1.push(block);
    return true;
}

void printCache() {
    cout << "    Cache State -> L1: [";
    queue<string> temp = L1;
    while (!temp.empty()) {
        cout << temp.front();
        temp.pop();
        if (!temp.empty()) cout << ", ";
    }
    cout << "] L2: [";
    temp = L2;
    while (!temp.empty()) {
        cout << temp.front();
        temp.pop();
        if (!temp.empty()) cout << ", ";
    }
    cout << "] L3: [";
    temp = L3;
    while (!temp.empty()) {
        cout << temp.front();
        temp.pop();
        if (!temp.empty()) cout << ", ";
    }
    cout << "]" << endl;
}

void roundRobin(vector<Task> tasks, int quantum) {
    queue<Task*> readyQueue;
    int cycle = 1;
    
    // Add all tasks to queue initially
    for (int i = 0; i < tasks.size(); i++) {
        readyQueue.push(&tasks[i]);
    }
    
    cout << "\n=== Round Robin Scheduler (Quantum=" << quantum << ") ===" << endl;
    
    while (!readyQueue.empty()) {
        Task* current = readyQueue.front();
        readyQueue.pop();
        
        cout << "\nCycle " << cycle << " - Running: " << current->id << endl;
        
        // Process memory blocks for this task
        for (string block : current->memBlocks) {
            int memoryCycles = 0;
            searchAndPromote(block, memoryCycles);
            printCache();
            cycle += memoryCycles;
        }
        
        // How many cycles this task runs this turn
        int runTime = min(quantum, current->remaining);
        current->remaining -= runTime;
        cycle += runTime;
        
        // If task not finished, add back to queue
        if (current->remaining > 0) {
            readyQueue.push(current);
        } else {
            cout << "  --> " << current->id << " COMPLETED at cycle " << cycle << endl;
        }
    }
    
    cout << "\n==============================" << endl;
    cout << "=== FINAL RESULTS ===" << endl;
    cout << "==============================" << endl;
    cout << "Total Cycles      : " << cycle-1 << endl;
    cout << "Tasks Completed   : " << tasks.size() << endl;
    cout << "Scheduler         : Round Robin (Quantum = " << quantum << ")" << endl;
    cout << "Total RAM Accesses: " << ramAccess << endl;
    cout << "==============================" << endl;
}



int main() {
    vector<Task> tasks = readTasks("input.txt");
    
    cout << "Tasks loaded:" << endl;
    for (Task t : tasks) {
        cout << "Task: " << t.id 
             << " Burst: " << t.burst 
             << " Mem: ";
        for (string m : t.memBlocks) {
            cout << m << " ";
        }
        cout << endl;
    }
    
    // Run Round Robin with quantum = 3
    roundRobin(tasks, 3);
    
    return 0;
}