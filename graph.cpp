#include <memory>
#include "graph.h"

void Graph::reset_graph() {
    this->MaxVertices = 0;
    this->NumEdges = 0;
    this->EdgeVertex.clear();
    this->VC_1.clear();
    this->VC_2.clear();
    this->VC_CNF.clear();
    for (int i = 0; i < MAX_VSIZE; ++i) {
        for (int j = 0; j < MAX_VSIZE; ++j) {
            this->Edge[i][j] = 0;
        }
    }
}

void Graph::insert_edge(int v1, int v2) {
    if (v1 == v2)
        return;
    if ((this->Edge[v1][v2] != 1) && (this->Edge[v2][v1] != 1)) {
        this->Edge[v1][v2] = this->Edge[v2][v1] = 1;
        this->NumEdges++;
    }
    this->EdgeVertex.push_back(v1);
    this->EdgeVertex.push_back(v2);
}

int rand_gen(int size) {
    srand(time(0));
    return rand() % size + 1;
}

void Graph::APPROX_VC_1() {
    vector<int> result;
    vector<int> vertex(this->MaxVertices, 0);
    vector<int> EdgeVertex = this->EdgeVertex;
    int NumEdges = this->NumEdges;
    for (int i : this->EdgeVertex) {
        vertex[i]++;
    }
    while (NumEdges != 0) {
        int MaxVertex = 0;
        int temp = 0;
        for (int i = 0; i < this->MaxVertices; i++) {
            if (temp < vertex[i]) {
                MaxVertex = i;
                temp = vertex[i];
                //vertex[i] = 0;
            }
        }
        result.push_back(MaxVertex);
        for (unsigned int i = 0; i < EdgeVertex.size(); i = i + 2) {
            if ((EdgeVertex[i] == MaxVertex) || (EdgeVertex[i+1] == MaxVertex)) {
                vertex[EdgeVertex[i]]--;
                vertex[EdgeVertex[i+1]]--;
                NumEdges--;
                EdgeVertex.erase(EdgeVertex.begin() + i);
                EdgeVertex.erase(EdgeVertex.begin() + i);
                i = i - 2;
            }
        }
    }
    sort(result.begin(), result.end());
    this->VC_1 = result;
    result.clear();
    vertex.clear();
    EdgeVertex.clear();
}

void Graph::APPROX_VC_2() {
    vector<int> result;
    vector<int> vertex(this->MaxVertices, 0);
    vector<int> EdgeVertex = this->EdgeVertex;
    int NumEdges = this->NumEdges;
    for (int i : this->EdgeVertex) {
        vertex[i]++;
    }

    while(NumEdges != 0) {
        int ran = rand_gen(NumEdges);
        int v1 = EdgeVertex[2 * ran - 2];
        int v2 = EdgeVertex[2 * ran - 1];
        result.push_back(v1);
        result.push_back(v2);
        NumEdges--;
        EdgeVertex.erase(EdgeVertex.begin() + (2 * ran - 2));
        EdgeVertex.erase(EdgeVertex.begin() + (2 * ran - 2));
        for (unsigned int i = 0; i < EdgeVertex.size(); i = i + 2) {
            if ((EdgeVertex[i] == v1) || (EdgeVertex[i+1] == v1) || (EdgeVertex[i] == v2) || (EdgeVertex[i+1] == v2)) {
                NumEdges--;
                EdgeVertex.erase(EdgeVertex.begin() + i);
                EdgeVertex.erase(EdgeVertex.begin() + i);
                i = i - 2;
            }
        }
    }
    sort(result.begin(), result.end());
    this->VC_2 = result;
    result.clear();
    vertex.clear();
    EdgeVertex.clear();
}

void Graph::CNF_SAT_VC() {
    vector<int> temp;
    vector<int> result;
    int l = 1;
    int h = this->MaxVertices;
    int NumVC;
    while (l <= h) {
        NumVC = floor((l + h) / 2);
        Solver s;
        vector<vector<Lit>> allLits(this->MaxVertices);
        allLits.clear();
        temp.clear();

        for(int i = 0; i < this->MaxVertices; i++) {
            for(int j = 0; j < NumVC; j++) {
                Lit li = mkLit(s.newVar());
                allLits[i].push_back(li);
            }
        }

        vec<Lit> literals;

        for(int i = 0; i < NumVC; i++) {
            literals.clear();
            for(int j = 0; j < this->MaxVertices; j++){
                literals.push(allLits[j][i]);
            }
            s.addClause(literals);
        }

        for (int i = 0; i < this->MaxVertices; i++) {
            for(int j = 0; j < NumVC; j++){
                for(int k = j + 1; k < NumVC; k++){
                    literals.clear();
                    literals.push(~allLits[i][j]);
                    literals.push(~allLits[i][k]);
                    s.addClause(literals);
                }
            }
        }

        for(int i = 0; i < NumVC; i++) {
            for(int j = 0; j < this->MaxVertices; j++){
                for(int k = j + 1; k< this->MaxVertices; k++){
                    literals.clear();
                    literals.push(~allLits[k][i]);
                    literals.push(~allLits[j][i]);
                    s.addClause(literals);
                }
            }
        }


        for(unsigned int i =0; i < (this->EdgeVertex.size()); i += 2) {
            literals.clear();
            for(int j = 0; j < NumVC; j++){
                literals.push(allLits[this->EdgeVertex[i]][j]);
                literals.push(allLits[this->EdgeVertex[i + 1]][j]);
            }
            s.addClause(literals);
        }
        literals.clear();

        bool res = s.solve();
        if (res) {
            for(int i = 0; i < this->MaxVertices; i++) {
                for(int j = 0; j < NumVC; j++) {
                    if (toInt(s.modelValue(allLits[i][j])) == 0) {
                        temp.push_back(i);
                    }
                }
            }
            h = NumVC - 1;
            result = temp;
        } else {
            l = NumVC + 1;
        }
    }
    sort(result.begin(), result.end());
    this->VC_CNF = result;
    result.clear();
    temp.clear();
}

/*
 * https://cse.iitkgp.ac.in/~palash/2018AlgoDesignAnalysis/SAT-3SAT.pdf
 *
 * http://vnsgu.ac.in/dept/publication/vnsgujst41july2015/25.pdf
 *
 * https://shodhganga.inflibnet.ac.in/bitstream/10603/119979/6/chapter%205.pdf
 */

void Graph::print_VC() {
    cout << "CNF-SAT-VC: ";
    if ((this->NumEdges != 0) && (!this->VC_CNF.empty())){
        for (unsigned int i = 0; i < this->VC_CNF.size() - 1; i++) {
            cout << this->VC_CNF[i] << ",";
        }
        cout << this->VC_CNF[this->VC_CNF.size() - 1] << "\n";
    } else if (this->VC_CNF.empty()) {
        cout << "timeout\n";
    } else {
        cout << endl;
    }
    cout << "APPROX-VC-1: ";
    if ((this->NumEdges != 0)  && (!this->VC_1.empty())){
        for (unsigned int i = 0; i < this->VC_1.size() - 1; i++) {
            cout << this->VC_1[i] << ",";
        }
        cout << this->VC_1[this->VC_1.size() - 1] << "\n";
    } else {
        cout << endl;
    }
    cout << "APPROX-VC-2: ";
    if ((this->NumEdges != 0)  && (!this->VC_2.empty())) {
        for (unsigned int i = 0; i < this->VC_2.size() - 1; i++) {
            cout << this->VC_2[i] << ",";
        }
        cout << this->VC_2[this->VC_2.size() - 1] << "\n";
    } else {
        cout << endl;
    }
}