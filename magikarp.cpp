#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <cmath>
#include <chrono>
#include <numeric>
#include <iomanip>
#include <random>
#include <algorithm>
#include <string>

// rastrear os elementos e seus sinais
struct Element {
    int64_t value;
    std::vector<std::pair<int64_t, int>> components; // Par de número e sinal (+1 ou -1)

    // elementos iniciais
    Element(int64_t v) : value(v) {
        components.emplace_back(v, +1);
    }

    // combinar elementos durante o algoritmo
    Element(int64_t v, const Element& e1, const Element& e2) : value(v) {
        // componentes do primeiro elemento com o mesmo sinal
        components.insert(components.end(), e1.components.begin(), e1.components.end());
        // inverte o sinal do segundo elemento
        for (const auto& comp : e2.components) {
            components.emplace_back(comp.first, -comp.second);
        }
    }

    // fila de prioridade
    bool operator<(const Element& other) const {
        return value < other.value;
    }
};

// Karmarkar-Karp com GRASP SA 
int64_t karmarkar_karp_with_grasp(const std::vector<int64_t>& numbers, std::vector<int>& solution, double alpha, std::mt19937& rng) {
    auto compare = [](const Element& a, const Element& b) { return a.value < b.value; };
    std::priority_queue<Element, std::vector<Element>, decltype(compare)> max_heap(compare);
   
    for (const auto& num : numbers) {
        max_heap.push(Element(num));
    }

    while (max_heap.size() > 1) {
        // remove o maior
        Element largest = max_heap.top();
        max_heap.pop();

        // RCL para o segundo 
        std::vector<Element> rcl;
        // seleciona da heap
        std::vector<Element> temp_elements;
        while (!max_heap.empty()) {
            temp_elements.push_back(max_heap.top());
            max_heap.pop();
        }

        // valor mínimo e máximo nos elementos temporários
        int64_t min_value = temp_elements.back().value; // Já está ordenado
        int64_t max_value = temp_elements.front().value;

        // alcance da RCL 
        int64_t threshold = max_value - static_cast<int64_t>(alpha * (max_value - min_value));

        for (const auto& elem : temp_elements) {
            if (elem.value >= threshold) {
                rcl.push_back(elem);
            } else {
                break; // Como está ordenado, podemos sair do loop
            }
        }

        // segundo elemento da RCL é aleatório
        std::uniform_int_distribution<std::size_t> dist(0, rcl.size() - 1);
        std::size_t index = dist(rng);
        Element second = rcl[index];

        // removeme o segundo 
        temp_elements.erase(temp_elements.begin() + index);

        // a diferença e cria um novo elemento
        int64_t difference = std::abs(largest.value - second.value);
        Element new_element(difference, largest, second);

        // demais voltam pra heap
        for (const auto& elem : temp_elements) {
            max_heap.push(elem);
        }

        // novo elemento na heap
        max_heap.push(new_element);
    }

    // o último elemento restante tem a diferença mínima e os componentes
    Element final_element = max_heap.top();
    max_heap.pop();

    // solução inicial
    solution.resize(numbers.size(), 0); // vetor de sinais (+1 ou -1)
    for (const auto& comp : final_element.components) {
        int64_t num = comp.first;
        int sign = comp.second;
        // Encontra o índice do número original
        auto it = std::find(numbers.begin(), numbers.end(), num);
        if (it != numbers.end()) {
            std::size_t idx = std::distance(numbers.begin(), it);
            solution[idx] = sign;
        }
    }

    return final_element.value;
}

// Simulated Annealing na busca local com resfriamento exponencial
void simulated_annealing(const std::vector<int64_t>& numbers, std::vector<int>& solution, int64_t& best_difference, std::mt19937& rng, double initial_temperature, double cooling_rate) {
    
    double temperature = initial_temperature;

    int64_t current_difference = 0;
    {
        int64_t sum1 = 0, sum2 = 0;
        for (std::size_t i = 0; i < numbers.size(); ++i) {
            if (solution[i] == +1) {
                sum1 += numbers[i];
            } else {
                sum2 += numbers[i];
            }
        }
        current_difference = sum1 - sum2;
    }

    std::vector<int> current_solution = solution;
    std::vector<int> best_solution = solution;

    // SA Loop
    while (temperature > 1e-6) { // temp min
        // nova solução vizinha trocando o sinal de um elemento aleatório
        std::uniform_int_distribution<std::size_t> dist(0, numbers.size() - 1);
        std::size_t idx = dist(rng);
        current_solution[idx] *= -1; 

        // diferença da nova solução incrementalmente
        int64_t delta_value = 2 * current_solution[idx] * numbers[idx];
        int64_t new_difference = current_difference + delta_value;

        // variação de energia
        int64_t delta = std::abs(new_difference) - std::abs(current_difference);

        // se aceita a nova 
        if (delta < 0 || (std::exp(-delta / temperature) > ((double)rng() / rng.max()))) {
            // caso sim
            current_difference = new_difference;
            solution = current_solution;

            // atualiza
            if (std::abs(current_difference) < best_difference) {
                best_difference = std::abs(current_difference);
                best_solution = solution;
            }
        } else {
            // caso não, reverte
            current_solution[idx] *= -1;
        }

        // diminui a temp (exp)
        temperature *= cooling_rate;
    }

    // atualiza a solução com a melhor encontrada
    solution = best_solution;
}

// ler a instância do arquivo
bool read_instance(const std::string& filename, std::vector<int64_t>& numbers) {
    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "Erro ao abrir o arquivo: " << filename << std::endl;
        return false;
    }

    int64_t N;
    if (!(infile >> N)) {
        std::cerr << "Erro ao ler o número de elementos N." << std::endl;
        return false;
    }

    numbers.reserve(N); 

    int64_t num;
    for (int64_t i = 0; i < N; ++i) {
        if (!(infile >> num)) {
            std::cerr << "Erro ao ler o elemento número " << i + 1 << "." << std::endl;
            return false;
        }
        numbers.push_back(num);
    }

    infile.close();
    return true;
}


int main(int argc, char* argv[]) {

    if (argc < 7) {
        std::cerr << "Uso: " << argv[0] << " <arquivo_instancia> <alpha> <iteracoes> <temperatura_inicial> <cooling_rate> <seed>" << std::endl;
        return 1;
    }


    std::string filename = argv[1];
    double alpha = std::stod(argv[2]); // Alpha para o GRASP
    int iterations = std::stoi(argv[3]); // Número de iterações do GRASP
    double initial_temperature = std::stod(argv[4]); // Temperatura inicial do SA
    double cooling_rate = std::stod(argv[5]); // Fator de resfriamento do SA
    int seed = std::stoi(argv[6]); // Seed 

    std::vector<int64_t> numbers;
    if (!read_instance(filename, numbers)) {
        return 1;
    }

    std::mt19937 rng(seed);

    int64_t best_difference = INT64_MAX;
    std::vector<int> best_solution(numbers.size(), +1); // Vetor de sinais (+1 ou -1)

    auto start = std::chrono::high_resolution_clock::now();

    // GRASP
    for (int i = 0; i < iterations; ++i) {
        std::vector<int> solution(numbers.size(), +1); // Solução atual

        // solução inicial 
        int64_t difference = karmarkar_karp_with_grasp(numbers, solution, alpha, rng);

        // SA
        simulated_annealing(numbers, solution, difference, rng, initial_temperature, cooling_rate);

        if (difference < best_difference) {
            best_difference = difference;
            best_solution = solution;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::vector<int64_t> subset1, subset2;
    for (std::size_t i = 0; i < numbers.size(); ++i) {
        if (best_solution[i] == +1) {
            subset1.push_back(numbers[i]);
        } else {
            subset2.push_back(numbers[i]);
        }
    }

    int64_t sum1 = std::accumulate(subset1.begin(), subset1.end(), int64_t(0));
    int64_t sum2 = std::accumulate(subset2.begin(), subset2.end(), int64_t(0));

    std::cout << "Diferença mínima possível entre as somas dos dois subconjuntos: " << std::abs(sum1 - sum2) << std::endl;

    std::cout << "Subconjunto 1: ";
    for (auto num : subset1) {
        std::cout << num << " ";
    }
    std::cout << "\nSubconjunto 2: ";
    for (auto num : subset2) {
        std::cout << num << " ";
    }
    std::cout << "\n";

     std::cout << "Soma do Subconjunto 1: " << sum1 << std::endl;
    std::cout << "Soma do Subconjunto 2: " << sum2 << std::endl;
    std::cout << "Diferença real entre as somas dos subconjuntos: " << std::abs(sum1 - sum2) << std::endl;
    std::cout << "Tempo de execução: " << std::fixed << std::setprecision(6) << duration.count() << " segundos" << std::endl;

    return 0;
}
