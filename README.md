# Magikarp

Projeto feito para a Disciplina PGC-UFF Otimização Multiobjetivo do Prof. Dr. Igor Machado Coelho

**Magikarp** é um projeto em C++ para resolver o problema de particionamento de números usando uma combinação dos algoritmos **Karmarkar-Karp**, **GRASP (Greedy Randomized Adaptive Search Procedure)** e **Simulated Annealing (SA)**. O objetivo é dividir um conjunto de números em dois subconjuntos com a menor diferença possível entre suas somas.

## Características

- **Algoritmo Karmarkar-Karp** para uma solução inicial de particionamento.
- **GRASP** para construção da solução com base em uma Lista Restrita de Candidatos (RCL).
- **Simulated Annealing** para refinar a solução e alcançar um ótimo local.
- **Suporte para entrada de dados via arquivo** para instâncias personalizadas.
- **Medição de tempo de execução** para avaliar a eficiência do algoritmo.

## Estrutura do Projeto

- **magikarp.cpp**: Código-fonte principal contendo o algoritmo de particionamento.
- **Compilação**: Utilize um compilador C++ para compilar o código (`g++` é recomendado).

## Como Compilar

Execute o seguinte comando no terminal:

```bash
g++ magikarp.cpp -o magikarp -std=c++11
