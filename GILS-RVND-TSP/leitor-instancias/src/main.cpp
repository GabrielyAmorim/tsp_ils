#include "Data.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <chrono> 

using namespace std;

struct solution{
    vector<int> sequence;
    double cost;
};

// RELACIONADO A CONTRUCAO()

struct insertionInfo{
    int noInserido; // cidade k a ser inserida
    int arestaRemovida; // aresta {i,j} onde k sera inserida
    double custo; // delta com a inserção de k
};

vector <insertionInfo> calcularCustoInsercao(solution& s, vector<int>& CL, Data& data){
    vector<insertionInfo> custoInsercao;

    // Percorre todas as arestas do tour atual e calcula o custo de inserir cada cidade k da CL entre as arestas
    for(int pos = 0; pos < (int)s.sequence.size() - 1; pos++){
        int i = s.sequence[pos];
        int j = s.sequence[pos + 1];

        for(int k : CL){ // Para cada cidade k na CL
            insertionInfo info;
            info.noInserido = k;
            info.arestaRemovida = pos;
            info.custo = data.getDistance(i, k) + data.getDistance(k, j) - data.getDistance(i, j);
            custoInsercao.push_back(info);
        }
    }

    return custoInsercao;
}

// Função Construção (GRASP)
solution construcao(Data& data){
    solution s;
    vector<int> CL; // Candidate List (cidades a serem inseridas)

    // Preenche a CL com todas as cidades
    for(int i = 1; i <= data.getDimension(); i++){
        CL.push_back(i);
    }

    random_shuffle(CL.begin(), CL.end()); //Embaralha a CL

    // Cria o subtour inicial com as quatro primeiras cidades  
    s.sequence.push_back(CL[0]);
    s.sequence.push_back(CL[1]);
    s.sequence.push_back(CL[2]);
    s.sequence.push_back(CL[3]);
    s.sequence.push_back(CL[0]);

    // Remove as cidades já usadas
    for(int i = 0; i < 4; i++){
        CL.erase(CL.begin());
    }

    // Enquanto houver cidades na CL, inserir a próxima cidade
    while(!CL.empty()){
        vector<insertionInfo> custoInsercao = calcularCustoInsercao(s, CL, data);

        // Organiza o custo em ordem crescente
        sort(custoInsercao.begin(), custoInsercao.end(),
            [](const insertionInfo& a, const insertionInfo& b){
                return a.custo < b.custo;
            });   
        
        // Selecionar aleatoriamente entre os melhores para não viciar o algoritmo
        double alpha = (double)rand() / RAND_MAX; // num entre 0 e 1
        int maxSelecionado = (int)ceil(alpha * custoInsercao.size());
        if(maxSelecionado == 0) maxSelecionado = 1;
        int selecionado = rand() % maxSelecionado;

        // Quem foi selecionado
        int k = custoInsercao[selecionado].noInserido;
        int pos = custoInsercao[selecionado].arestaRemovida + 1;

        // Insere a escolhida
        s.sequence.insert(s.sequence.begin() + pos, k);    
 
        // Remove a escolhida da CL
        CL.erase(find(CL.begin(), CL.end(), k));
    }

    // Custo completo da rota (custo final)
    s.cost = 0.0;
    for(int i = 0; i < (int)s.sequence.size() - 1; i++){
        s.cost += data.getDistance(s.sequence[i], s.sequence[i + 1]); 
    }

    return s;
}

// RELACIONADO A BUSCALOCAL()

// Troca a posição de duas arestas na rota
bool bestImprovementSwap(solution& s, Data& data){
    double bestDelta = 0.0;
    int best_i = 0, best_j = 0;
    int n = s.sequence.size();

    for(int i = 1; i < n - 1; i++){
        for(int j = i + 2; j < n - 1; j++){
            double delta = - data.getDistance(s.sequence[i - 1], s.sequence[i])
                           - data.getDistance(s.sequence[i], s.sequence[i + 1])
                           - data.getDistance(s.sequence[j - 1], s.sequence[j])
                           - data.getDistance(s.sequence[j], s.sequence[j + 1])
                           + data.getDistance(s.sequence[j - 1], s.sequence[i])
                           + data.getDistance(s.sequence[i], s.sequence[j + 1])
                           + data.getDistance(s.sequence[i - 1], s.sequence[j])
                           + data.getDistance(s.sequence[j], s.sequence[i + 1]);

            if(delta < bestDelta){
                bestDelta = delta;
                best_i = i;
                best_j = j; 
            }               
        }
    }

    // Aplica o movimento
     if(bestDelta < 0){
        swap(s.sequence[best_i], s.sequence[best_j]);
        s.cost += bestDelta;
        return true;
    }

    return false;
}


// Remove duas arestas não adjacentes e reconecta os segmentos invertidamente
bool bestImprovement2Opt(solution& s, Data& data){
    double bestDelta = 0.0;
    int best_i = 0, best_j = 0;
    int n = s.sequence.size();


    for(int i = 1; i < n - 1; i++){
        for(int j = i + 2; j < n - 1; j++){
            double delta = - data.getDistance(s.sequence[i], s.sequence[i + 1])
                           - data.getDistance(s.sequence[j], s.sequence[j + 1])
                           + data.getDistance(s.sequence[i], s.sequence[j])
                           + data.getDistance(s.sequence[i + 1], s.sequence[j + 1]);

            if(delta < bestDelta){
                bestDelta = delta;
                best_i = i;
                best_j = j;
            }               
        }
    }

    // Aplicando o movimento
    if(bestDelta < 0){
        reverse(s.sequence.begin() + best_i + 1, s.sequence.begin() + best_j + 1);
        s.cost += bestDelta;
        return true;
    }

    return false;
}

// Remove um bloco de tamanho l e insere-o em outra posição
bool bestImprovementOrOpt(solution& s, Data& data, int l){
    double bestDelta = 0.0;
    int best_i = 0, best_j = 0;
    int n = s.sequence.size();

    for(int i = 1; i <= n - l - 1; i++){
        for(int j = 1; j <= n - l; j++){
            
            // Verifica sobreposição
            if(j >= i && j <= i + l - 1) continue;
            
            // Pula movimentos sem efeito (adjacentes)
            if(j == i - 1 || j == i + l) continue;
            
            double delta = - data.getDistance(s.sequence[i - 1], s.sequence[i])
                           - data.getDistance(s.sequence[i + l - 1], s.sequence[i + l])
                           - data.getDistance(s.sequence[j - 1], s.sequence[j])
                           + data.getDistance(s.sequence[i - 1], s.sequence[i + l])
                           + data.getDistance(s.sequence[j - 1], s.sequence[i])
                           + data.getDistance(s.sequence[i + l - 1], s.sequence[j]);

            if(delta < bestDelta){
                bestDelta = delta;
                best_i = i;
                best_j = j;
            }               
        }
    }

    if(bestDelta < 0){
        // Extrai o bloco
        vector<int> bloco;
        for(int k = 0; k < l; k++){
            bloco.push_back(s.sequence[best_i + k]);
        }

        s.sequence.erase(s.sequence.begin() + best_i, s.sequence.begin() + best_i + l);
        
        int posInsercao;
        if(best_j < best_i){
            posInsercao = best_j;      // Inserir antes
        } 
        else {
            posInsercao = best_j - l;  // Inserir depois
        }
        
        // Insere o bloco
        s.sequence.insert(s.sequence.begin() + posInsercao, bloco.begin(), bloco.end());
        s.cost += bestDelta;
        return true;
    }

    return false;
}

// Função buscaLocal()
void buscaLocal(solution& s, Data& data){
    vector<int> NL = {1, 2, 3, 4, 5}; // Lista de vizinhanças (movimentos)
    bool improved = false;

    while(!NL.empty()){
        int v = rand() % NL.size(); // Escolhe uma vizinhança aleatória para testar

        switch(NL[v]){
            case 1:
                improved = bestImprovementSwap(s, data); // SWAP
                break;
            case 2:
                improved = bestImprovement2Opt(s, data); // 2-OPT
                break;
            case 3:
                improved = bestImprovementOrOpt(s, data, 1); // REINSERTION
                break;
            case 4:
                improved = bestImprovementOrOpt(s, data, 2); // OR-OPT-2
                break;
            case 5:
                improved = bestImprovementOrOpt(s, data, 3); // OR-OPT-3
                break;           
        }

        // Atualização da lista
        if(improved){
            NL = {1, 2, 3, 4, 5}; // Se melhorou, reinicia a lista
        } else {
            NL.erase(NL.begin() + v); // Senão, remove a vizinhança testada
        }
    }
}

solution perturbacao(solution best, Data& data){
    solution copia = best;
    int n = copia.sequence.size() - 1;
    
    // Tamanhos dos blocos
    int maxBlock = max(2, (int)ceil(n / 10.0));
    int tamanho1 = 2 + rand() % (maxBlock - 1);
    int tamanho2 = 2 + rand() % (maxBlock - 1);
    
    // Calcula pos1 
    int maxPos1 = n - tamanho1 - tamanho2 - 3;
    int pos1 = 1 + rand() % maxPos1;
    
    // Calcula pos2 
    int minPos2 = pos1 + tamanho1 + 1;
    int maxPos2 = n - tamanho2 - 1;
    int pos2 = minPos2 + rand() % (maxPos2 - minPos2 + 1);
    
    // Aplica o movimento 
    vector<int> bloco1(copia.sequence.begin() + pos1, copia.sequence.begin() + pos1 + tamanho1);
    vector<int> bloco2(copia.sequence.begin() + pos2, copia.sequence.begin() + pos2 + tamanho2);
    
    // Constroi nova sequência
    vector<int> novaSequencia;
    novaSequencia.insert(novaSequencia.end(), copia.sequence.begin(), copia.sequence.begin() + pos1);
    novaSequencia.insert(novaSequencia.end(), bloco2.begin(), bloco2.end());
    novaSequencia.insert(novaSequencia.end(), copia.sequence.begin() + pos1 + tamanho1, copia.sequence.begin() + pos2);
    novaSequencia.insert(novaSequencia.end(), bloco1.begin(), bloco1.end());
    novaSequencia.insert(novaSequencia.end(), copia.sequence.begin() + pos2 + tamanho2, copia.sequence.end());
    
    copia.sequence = novaSequencia;

    // Recalcula o custo
    copia.cost = 0.0;
    for (int i = 0; i < n - 1; i++){
        copia.cost += data.getDistance(copia.sequence[i], copia.sequence[i + 1]);
    }
    copia.cost += data.getDistance(copia.sequence[n - 1], copia.sequence[0]);
    
    return copia;
}

solution ILS(int maxIter, int maxIterIls, Data& data){
    solution bestOfAll;
    bestOfAll.cost = 1e9;

    for(int i = 0; i < maxIter; i++){
        solution s = construcao(data);

        solution best = s;
        int iterIls = 0;

        while(iterIls <= maxIterIls){
            buscaLocal(s, data);
            
            if(s.cost < best.cost){
                best = s;
                iterIls = 0;
            }

            s = perturbacao(best, data);
            
            iterIls++;
        }

        if (best.cost < bestOfAll.cost){
            bestOfAll = best;
        }
    }

    return bestOfAll;
}

int main(int argc, char** argv){
    auto start = chrono::high_resolution_clock::now();

    srand(time(NULL));

    auto data = Data(argc, argv[1]);
    data.read();
    size_t n = data.getDimension();

    cout << "Dimension: " << n << endl;

    // Parametros do ILS
    int maxIter = 50;
    int maxIterILS;
    if (n >= 150){
        maxIterILS = n / 2;
    } else{
        maxIterILS = n;
    }

    cout << "\nExecutando ILS para " << n << " cidades..." << endl;
    solution bestSolution = ILS(maxIter, maxIterILS, data);
    
    cout << "Melhor solucao encontrada: ";
    for (int i = 0; i < (int)n; i++){
        cout << bestSolution.sequence[i] << " -> ";
    }
    cout << bestSolution.sequence[0] << endl;
    cout << "Custo da melhor solucao: " << bestSolution.cost << endl;

    auto end = chrono::high_resolution_clock::now();

    chrono::duration<double> duration_sec = end - start;
    cout << "\nTempo de execucao: " << duration_sec.count() << " segundos" << endl;

    return 0;
}