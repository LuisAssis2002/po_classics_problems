/*---------------- File: main.cpp  ---------------------+
|Modelo PLI - Problema do Transporte (PT)               |
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

struct vertice {
    int id, w; //id do vertice e sua necessidade/recurso
};

struct aresta {
	int w; //capacidade
};

//Conjuntos do Problema
int O; //Quantidade de origens
vector<vertice> origens; //Conjunto das origens
vector<vertice> demandas; //Conjunto das demandas
vector<vector<aresta>> arestas; //Conjunto das arestas
int D; //Quantidade de demandas

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
		for( j = 0; j < D; j++ ){
			x[i].add(IloIntVar(env, 0, 1000));
			numberVar++;
		}
	}

	//Definicao do ambiente modelo ------------------------------------------
	IloModel model ( env );
	
	//Definicao do ambiente expressoes, para os somatorios ---------------------------------
	IloExpr sum(env); /// Expression for Sum
	IloExpr sum2(env); /// Expression for Sum2

	//FUNCAO OBJETIVO ---------------------------------------------
	sum.clear();
	for( i = 0; i < O; i++ ){
		for( j = 0; j < D; j++ ){
			sum += (arestas[i][j].w * x[i][j]);
		}
	}

	//Modelo de Minimizacao
	model.add(IloMinimize(env, sum)); //Minimizacao

	//RESTRICOES ---------------------------------------------	
	 
	//Restrições - Respeito das demandas
	for( i = 0; i < D; i++ ){
		sum.clear();
		for( j = 0; j < O; j++ ){
			sum += x[j][i];
		}
		model.add(sum >= demandas[i].w); 
		numberRes++;
	}		

	//Restrições - Respeito das origens
	for( i = 0; i < O; i++ ){
		sum.clear();
		for( j = 0; j < D; j++ ){
			sum += x[i][j];
		}
		model.add(sum <= origens[i].w); 
		numberRes++;
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
			for( j = 0; j < D; j++ ){
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
    
	int i, o, d, w;
	cin >> O >> D;

	origens.resize(O);
	demandas.resize(D);
	arestas.resize(O);

	for(i=0; i<O; i++) {
		arestas[i].resize(D);
	}

	for(i=0; i<O; i++) {
		for(int l=0; l<D; l++) {
			arestas[i][l].w = 0;
		}	
	}

	for(i=0; i<O; i++){
		cin >> origens[i].w;
	} 
	for(i=0; i<D; i++){
		cin >> demandas[i].w;
	}
	while(!cin.eof()) {
		cin >> o >> d >> w;
		arestas[o][d].w = w;
	}





	printf("Verificacao da leitura dos dados:\n");
	printf("Num. de origens: %d\n", O);
	printf("Num. de demandas: %d\n", D);
	printf("Local: id - Destino: id - Capacidade\n");
	for(i=0; i<O; i++) {
		for(int l=0; l<D; l++) {
        	if(arestas[i][l].w != 0) printf("origem: %d - destino: %d - capacidade: %d\n", i, l, arestas[i][l].w);
		}
	}

	cplex();

    return 0;
}
