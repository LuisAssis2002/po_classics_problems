/*---------------- File: main.cpp  ---------------------+
|Modelo PLI - Problema de Fluxo Máximo (PFM)            |
|					      		                        |
|					      		                        |
| Implementado por: LUIS ARTHUR DE ASSIS MORAES  	    |
| 					ALEX DE ANDRADE SOARES  	        |
+-------------------------------------------------------+ */

#include <bits/stdc++.h>
#include <ilcplex/ilocplex.h>

using namespace std;
ILOSTLBEGIN //MACRO - "using namespace" for ILOCPEX

//CPLEX Parameters
#define CPLEX_TIME_LIM 3600 //3600 segundos
//#define CPLEX_COMPRESSED_TREE_MEM_LIM 8128 //8GB
//#define CPLEX_WORK_MEM_LIM 4096 //4GB
//#define CPLEX_VARSEL_MODE 0
/*
* VarSel Modes:
* -1 Branch on variable with minimum infeasibility
* 0 Branch variable automatically selected
* 1 Branch on variable with maximum infeasibility
* 2 Branch based on pseudo costs
* 3 Strong branching
* 4 Branch based on pseudo reduced costs
*
* Default: 0
*/



struct aresta {
	int w;	//capacidade do caminho
};

//Conjuntos do Problema
int O; //Quantidade de Vertices
vector<vector<aresta>> arestas; //Conjunto de caminhos
int D; //id do vertice origem
int F;	//id do vertice destino

void cplex(){
    //CPLEX
	IloEnv env; //Define o ambiente do CPLEX

	//Variaveis --------------------------------------------- 
	int i, j, k; //Auxiliares
	int numberVar = 0; //Total de Variaveis
	int numberRes = 0; //Total de Restricoes


	//---------- MODELAGEM ---------------
	//Definicao - Variaveis de Decisao 2 dimensoes (x_ij) não binárias (discretas)
	IloArray<IloNumVarArray> x(env);
	for( i = 0; i < O; i++ ){
		x.add(IloNumVarArray(env));
		for( j = 0; j < O; j++ ){
			x[i].add(IloIntVar(env, 0, IloInfinity));
			numberVar++;
		}
	}

	//Definicao do ambiente modelo ------------------------------------------
	IloModel model ( env );
	
	//Definicao do ambiente expressoes, para os somatorios ---------------------------------
	IloExpr sum(env); /// Expression for Sum
	IloExpr sum2(env); /// Expression for Sum2

	//FUNCAO OBJETIVO ---------------------------------------------
	i = D;
	sum.clear();
	for( j = 0; j < O; j++ ){
		sum += x[i][j];
	}
	model.add(IloMaximize(env, sum)); //Maximizacao

	//RESTRICOES ---------------------------------------------	
	 
	//Restrições de ida e volta
	for( i = 0; i < O; i++ ){
		if ((i != D) && (i != F)) {
			sum.clear();
			for( j = 0; j < O; j++ ){
				sum += x[i][j];
			}
			for( j = 0; j < O; j++ ){
				sum -= x[j][i];
			}
			model.add(sum == 0); 
			numberRes++;
		}
	}		
	
	//Restrições de capacidade
	for( i = 0; i < O; i++ ){
		for( j = 0; j < O; j++ ){
			sum.clear();
			sum += x[i][j];
			model.add(sum <= arestas[i][j].w); 
			numberRes++;
		}
	}

	//------ EXECUCAO do MODELO ----------
	time_t timer, timer2;
	IloNum value, objValue;
	double runTime;
	string status;
	
	//Informacoes ---------------------------------------------	
	printf("--------Informacoes da Execucao:----------\n\n");
	printf("#Var: %d\n", numberVar);
	printf("#Restricoes: %d\n", numberRes);
	cout << "Memory usage after variable creation:  " << env.getMemoryUsage() / (1024. * 1024.) << " MB" << endl;
	
	IloCplex cplex(model);
	cout << "Memory usage after cplex(Model):  " << env.getMemoryUsage() / (1024. * 1024.) << " MB" << endl;

	//Setting CPLEX Parameters
	cplex.setParam(IloCplex::TiLim, CPLEX_TIME_LIM);
	//cplex.setParam(IloCplex::TreLim, CPLEX_COMPRESSED_TREE_MEM_LIM);
	//cplex.setParam(IloCplex::WorkMem, CPLEX_WORK_MEM_LIM);
	//cplex.setParam(IloCplex::VarSel, CPLEX_VARSEL_MODE);

	time(&timer);
	cplex.solve();//COMANDO DE EXECUCAO
	time(&timer2);
	
	//cout << "Solution Status: " << cplex.getStatus() << endl;
	//Results
	bool sol = true;

	switch(cplex.getStatus()){
		case IloAlgorithm::Optimal: 
			status = "Optimal";
			break;
		case IloAlgorithm::Feasible: 
			status = "Feasible";
			break;
		default: 
			status = "No Solution";
			sol = false;
	}

	cout << endl << endl;
	cout << "Status da FO: " << status << endl;

	if(sol){ 

		//Results
		//int Nbin, Nint, Ncols, Nrows, Nnodes, Nnodes64;
		objValue = cplex.getObjValue();
		runTime = difftime(timer2, timer);
		//Informacoes Adicionais
		//Nbin = cplex.getNbinVars();
		//Nint = cplex.getNintVars();
		//Ncols = cplex.getNcols();
		//Nrows = cplex.getNrows();
		//Nnodes = cplex.getNnodes();
		//Nnodes64 = cplex.getNnodes64();
		//float gap; gap = cplex.getMIPRelativeGap();
		
		cout << "Variaveis de decisao: " << endl;
		for( i = 0; i < O; i++ ){
			for( j = 0; j < O; j++ ){
				value = IloRound(cplex.getValue(x[i][j]));
				if(value != 0) printf("x[%d, %d]: %.0lf\n", i, j, value);
			}
		}
		printf("\n");
		
		cout << "Funcao Objetivo Valor = " << objValue << endl;
		printf("..(%.6lf seconds).\n\n", runTime);

	}else{
		printf("No Solution!\n");
	}

	//Free Memory
	cplex.end();
	sum.end();
	sum2.end();

	cout << "Memory usage before end:  " << env.getMemoryUsage() / (1024. * 1024.) << " MB" << endl;
	env.end();
}


int main() {
    
	int i, o, d, w, n_rotas = 0;
	cin >> O >> D >> F;

	arestas.resize(O);

	for(i=0; i<O; i++) {
		arestas[i].resize(O);
	}

	for(i=0; i<O; i++) {
		for(int l=0; l<O; l++) {
			arestas[i][l].w = 0;
		}	
	}

	while(!cin.eof()) {
		cin >> o >> d >> w;
		arestas[o][d].w = w;
		n_rotas++;
	}





	printf("Verificacao da leitura dos dados:\n");
	printf("Num. de locais: %d\n", O);
	printf("Num. de rotas: %d\n", n_rotas);
	printf("origem: id - destino: id - capacidade\n");
	for(i=0; i<O; i++) {
		for(int l=0; l<O; l++) {
        	if(arestas[i][l].w != 0) printf("origem: %d - destino: %d - capacidade: %d\n", i, l, arestas[i][l].w);
		}
	}

	cplex();

    return 0;
}
