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

    // Cria o subtour inicial com as tres primeiras cidades  
    s.sequence.push_back(CL[0]);
    s.sequence.push_back(CL[1]);
    s.sequence.push_back(CL[2]);
    s.sequence.push_back(CL[0]);

    // Remove as cidades já usadas
    for(int i = 0; i < 3; i++){
        CL.erase(CL.begin());
    }

    // Enquanto houver cidades na CL, inserir a próxima cidade
    while(!CL.empty()){
        // Custo de inserir cada cidade
        vector<insertionInfo> custoInsercao = calcularCustoInsercao(s, CL, data);

        // Organiza o custo em ordem crescente
        sort(custoInsercao.begin(), custoInsercao.end(),
            [](const insertionInfo& a, const insertionInfo& b){
                return a.custo < b.custo;
            });   
        
        // Selecionar aleatoriamente entre os melhores para não viciar o algoritmo
        double alpha = (double)rand() / RAND_MAX; //num entre 0 e 1
        int maxSelecionado = (int)ceil(alpha * custoInsercao.size());
        if(maxSelecionado == 0) maxSelecionado = 1;
        int selecionado = rand() % maxSelecionado;

        // Quem foi selecionado
        int k = custoInsercao[selecionado].noInserido;
        int pos = custoInsercao[selecionado].arestaRemovida + 1;

        // Insere a escolhida
        s.sequence.insert(s.sequence.begin() + pos, k);    
 
        // Remove a escolhida da CL
        auto it = find(CL.begin(), CL.end(), k);
        if(it != CL.end()){
            CL.erase(it);
        }
    }

    // Recalcula o custo completo da rota (custo final)
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

    // A cidade inicial e final é fixa
    for(int i = 1; i < n - 2; i++){
        for(int j = i + 1; j < n - 1; j++){
            double delta = - data.getDistance(s.sequence[i - 1], s.sequence[i])
                           - data.getDistance(s.sequence[i], s.sequence[i + 1])
                           + data.getDistance(s.sequence[i - 1], s.sequence[j])
                           + data.getDistance(s.sequence[j], s.sequence[i + 1])
                           - data.getDistance(s.sequence[j - 1], s.sequence[j])
                           - data.getDistance(s.sequence[j], s.sequence[j + 1])
                           + data.getDistance(s.sequence[j - 1], s.sequence[i])
                           + data.getDistance(s.sequence[i], s.sequence[j + 1]);

            if(delta < bestDelta){
                bestDelta = delta;
                best_i = i;
                best_j = j; 
            }               
        }
    }

    // Aplica o movimento com segurança, evitando loops infinitos
    if(bestDelta < 0){
        vector<int> teste = s.sequence; // Cópia da rota para testar o movimento
        swap(teste[best_i], teste[best_j]);

        double novoCusto = 0;
        for(int k = 0; k < n - 1; k++){
            novoCusto += data.getDistance(teste[k], teste[k + 1]);
        }

        // Só aplica se realmente melhorar
        if(novoCusto < s.cost){
            s.sequence = teste;
            s.cost = novoCusto;
            return true;
        }
    }
    return false;
}

// Remove duas arestas não adjacentes e reconecta os segmentos invertidamente
bool bestImprovement2Opt(solution& s, Data& data){
    double bestDelta = 0.0;
    int best_i = 0, best_j = 0;
    int n = s.sequence.size();

    if(n < 4) return false;

    // i não pode ser ultimo nem penultimo
    for(int i = 1; i < n - 2; i++){
        // j tem que ser pelo mennos i + 2 para evitar arestas adjacentes
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
        vector<int> teste = s.sequence; // Cópia teste
        reverse(teste.begin() + best_i + 1, teste.begin() + best_j + 1);

        double novoCusto = 0;
        for(int k = 0; k < n - 1; k++){
            novoCusto += data.getDistance(teste[k], teste[k + 1]);
        }

        if(novoCusto < s.cost){
            s.sequence = teste;
            s.cost = novoCusto;
            return true;
        }
    }
    return false;
}

// Remove um bloco de tamanho l e insere-o em outra posição
bool bestImprovementOrOpt(solution& s, Data& data, int l){
    double bestDelta = 0.0;
    int best_i = -1, best_j = -1;
    int n = s.sequence.size();

    if(n < l + 3) return false;

    for(int i = 1; i < n - l - 1; i++){  // i vai até n-l-2
        for(int j = 1; j < n - l - 1; j++){  // j vai até n-l-2
            
            // Verifica se o bloco não se sobrepõe
            if(j >= i && j <= i + l - 1) continue;
            if(i >= j && i <= j + l - 1) continue;
            
            // Proteção: verifica índices
            if(i + l >= n || j + l >= n) continue;
            if(i - 1 < 0 || j - 1 < 0) continue;

            double delta = - data.getDistance(s.sequence[i - 1], s.sequence[i])
                           - data.getDistance(s.sequence[j - 1], s.sequence[j])
                           - data.getDistance(s.sequence[j + l - 1], s.sequence[j + l])
                           + data.getDistance(s.sequence[i - 1], s.sequence[j])
                           + data.getDistance(s.sequence[j + l - 1], s.sequence[i])
                           + data.getDistance(s.sequence[j - 1], s.sequence[j + l]);

            if(delta < bestDelta){
                bestDelta = delta;
                best_i = i;
                best_j = j;
            }               
        }
    }

    // Aplica o movimento
    if(bestDelta < 0 && best_i != -1 && best_j != -1){
        vector<int> teste = s.sequence;
        
        if(best_j + l > n || best_i + l > n) return false;
        
        vector<int> bloco;
        for(int k = 0; k < l; k++){
            bloco.push_back(teste[best_j + k]);
        }

        teste.erase(teste.begin() + best_j, teste.begin() + best_j + l);

        int posInsercao = best_i;
        if(best_j < best_i){
            posInsercao = best_i - l;
        }
        
        if(posInsercao < 0 || posInsercao > (int)teste.size()) return false;

        teste.insert(teste.begin() + posInsercao, bloco.begin(), bloco.end());

        if((int)teste.size() != n) return false;
        if(teste.front() != teste.back()) return false;

        double novoCusto = 0;
        for(int k = 0; k < n - 1; k++){
            novoCusto += data.getDistance(teste[k], teste[k + 1]);
        }

        if(novoCusto < s.cost){
            s.sequence = teste;
            s.cost = novoCusto;
            return true;
        }
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
    int n = copia.sequence.size();
    int tamanho1, tamanho2;
    
    // Tamanho dos blocos
    if (n <= 20){
        tamanho1 = 2 + rand() % 2;
        tamanho2 = 2 + rand() % 2;
    } else {
        int maxBlock = max(2, n / 10);
        tamanho1 = 2 + rand() % (maxBlock - 1);
        tamanho2 = 2 + rand() % (maxBlock - 1);
    }

    // Posicao do primeiro bloco
    int maxPos1 = n - tamanho1 - 3;
    if (maxPos1 < 1) {
        return copia;
    }
    
    int pos1 = 1 + rand() % maxPos1;
    int espacoRestante = n - pos1 - tamanho1 - tamanho2 - 2;
    
    // Se não houver espaço, tenta com pos1 menor
    if (espacoRestante < 1) {
        int novoMax = max(1, n - tamanho1 - tamanho2 - 4);
        if (novoMax < 1) {
            return copia;
        }
        pos1 = 1 + rand() % novoMax;
        espacoRestante = n - pos1 - tamanho1 - tamanho2 - 2;
        if (espacoRestante < 1) {
            return copia;
        }
    }
    
    // Posicao do segundo bloco
    int pos2 = pos1 + tamanho1 + 1 + rand() % espacoRestante;

    // Verifica se os blocos não se sobrepõem e não tocam no final
    if (pos1 + tamanho1 >= pos2) {
        return copia;
    }
    if (pos2 + tamanho2 >= n - 1) {
        return copia;
    }

    // Extrai o bloco1
    vector<int> bloco1;
    for (int i = 0; i < tamanho1; i++) {
        bloco1.push_back(copia.sequence[pos1 + i]);
    }
    
    // Extrai o bloco2
    vector<int> bloco2;
    for (int i = 0; i < tamanho2; i++) {
        bloco2.push_back(copia.sequence[pos2 + i]);
    }

    // Remove bloco1 e insere bloco2 no lugar
    copia.sequence.erase(copia.sequence.begin() + pos1, copia.sequence.begin() + pos1 + tamanho1);
    copia.sequence.insert(copia.sequence.begin() + pos1, bloco2.begin(), bloco2.end());
    
    // Ajusta pos2 (pode ter mudado devido à remoção/inserção do bloco1)
    int pos2Ajustado;
    if (pos2 > pos1) {
        pos2Ajustado = pos2 - tamanho1 + tamanho2;
    } else {
        pos2Ajustado = pos2;
    }
    
    // Remove bloco2 (na posição ajustada) e insere bloco1 no lugar
    copia.sequence.erase(copia.sequence.begin() + pos2Ajustado, copia.sequence.begin() + pos2Ajustado + tamanho2);
    copia.sequence.insert(copia.sequence.begin() + pos2Ajustado, bloco1.begin(), bloco1.end());

    // Garante que a rota volta à origem
    copia.sequence.back() = copia.sequence[0];

    // Verifica se houve mudança
    bool mudou = false;
    for (int i = 0; i < n; i++) {
        if (copia.sequence[i] != best.sequence[i]) {
            mudou = true;
            break;
        }
    }
    
    // Se não mudou, retorna a solução original
    if (!mudou) {
        return best;
    }

    // Recalcula o custo da nova rota
    copia.cost = 0.0;
    for (int i = 0; i < (int)copia.sequence.size() - 1; i++) {
        copia.cost += data.getDistance(copia.sequence[i], copia.sequence[i + 1]);
    }
    
    return copia;
}

solution ILS(int maxIter, int maxIterIls, Data& data){
    solution bestOfAll;
    bestOfAll.cost = 1e9;

    for(int i = 0; i < maxIter; i++){
        solution s = construcao(data);
        
        if(s.sequence.size() != data.getDimension() + 1){
            continue;
        }
        
        solution best = s;
        int iterIls = 0;

        while(iterIls <= maxIterIls){
            bool valida = true;
            
            // Verifica se o primeiro e último são iguais
            if(s.sequence.front() != s.sequence.back()){
                valida = false;
            }
            
            // Verifica se todas as cidades de 1 a n estão presentes (sem duplicatas)
            if(valida){
                vector<int> check = s.sequence;
                check.pop_back(); // Remove a cidade de retorno (último elemento)
                sort(check.begin(), check.end());
                
                for (int j = 0; j < (int)check.size(); j++) {
                    if(check[j] != j + 1){
                        valida = false;
                        break;
                    }
                }
            }
            
            if (!valida) {
                s = best;
                iterIls++;
                continue;
            }

            buscaLocal(s, data);
            
            if(s.cost < best.cost){
                best = s;
                iterIls = 0;
            }

            s = perturbacao(best, data);
            
            // Verifica se a solução perturbada é válida
            if(s.sequence.size() != data.getDimension() + 1){
                s = best;
                continue;
            }
            
            iterIls++;
        }

        if (best.cost < bestOfAll.cost){
            bestOfAll = best;
        }
    }

    return bestOfAll;
}

int main(int argc, char** argv) {
    auto start = chrono::high_resolution_clock::now();

    srand(time(NULL));

    auto data = Data(argc, argv[1]);
    data.read();
    size_t n = data.getDimension();

    cout << "Dimension: " << n << endl;

    // Parametros do ILS
    int maxIter = 50;
    int maxIterILS;
    if (n >= 150) {
        maxIterILS = n / 2;
    } else {
        maxIterILS = n;
    }

    cout << "\nExecutando ILS para " << n << " cidades..." << endl;
    solution bestSolution = ILS(maxIter, maxIterILS, data);
    
    cout << "Melhor solucao encontrada: ";
    for (int i = 0; i < (int)n; i++) {
        cout << bestSolution.sequence[i] << " -> ";
    }
    cout << bestSolution.sequence[0] << endl;
    cout << "Custo da melhor solucao: " << bestSolution.cost << endl;

    auto end = chrono::high_resolution_clock::now();

    chrono::duration<double> duration_sec = end - start;
    cout << "\nTempo de execucao: " << duration_sec.count() << " segundos" << endl;

    return 0;
}