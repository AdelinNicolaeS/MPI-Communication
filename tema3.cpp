#include <fstream>
#include <iostream>
#include <mpi.h>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

int min(int a, int b) { return a < b ? a : b; }

string get_filename(int rank) { return "cluster" + to_string(rank) + ".txt"; }

int get_cluster(int key, unordered_map<int, vector<int>> graph) {
  for (int el : graph[0]) {
    if (el == key) {
      return 0;
    }
  }
  for (int el : graph[1]) {
    if (el == key) {
      return 1;
    }
  }
  for (int el : graph[2]) {
    if (el == key) {
      return 2;
    }
  }
  return -1;
}

int main(int argc, char *argv[]) {
  // vom avea graph[0], graph[1], graph[2]
  unordered_map<int, vector<int>> graph;
  int rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Procesare date aferente fiecarui cluster
  if (rank < 3) {
    ifstream fin(get_filename(rank));
    int count_numbers;
    vector<int> current_cluster;
    int worker, i;

    fin >> count_numbers;
    for (i = 0; i < count_numbers; i++) {
      fin >> worker;
      current_cluster.push_back(worker);
    }
    graph[rank] = current_cluster;

    fin.close();
  }

  // Informarea tuturor proceselor
  if (rank == 2) {
    vector<int> current_cluster = graph[rank];
    int cluster_size = current_cluster.size();

    // trimite clusterul sau catre 0
    MPI_Send(&cluster_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    cout << "M(2,0)" << endl;
    MPI_Send(&current_cluster[0], cluster_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
    cout << "M(2,0)" << endl;

    // trimite clusterul sau catre 1
    MPI_Send(&cluster_size, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    cout << "M(2,1)" << endl;
    MPI_Send(&current_cluster[0], cluster_size, MPI_INT, 1, 0, MPI_COMM_WORLD);
    cout << "M(2,1)" << endl;

    // primeste clusterul de la 0
    int cluster0_len;
    MPI_Recv(&cluster0_len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    vector<int> cluster0(cluster0_len, 0);
    MPI_Recv(&cluster0[0], cluster0_len, MPI_INT, 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    // primeste clusterul de la 1
    int cluster1_len;
    MPI_Recv(&cluster1_len, 1, MPI_INT, 1, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    vector<int> cluster1(cluster1_len, 0);
    MPI_Recv(&cluster1[0], cluster1_len, MPI_INT, 1, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    // isi completeaza topologia
    graph[0] = cluster0;
    graph[1] = cluster1;

    // bonus: trimite clusterul 1 catre 0
    MPI_Send(&cluster1_len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    cout << "M(2,0)" << endl;
    MPI_Send(&cluster1[0], cluster1_len, MPI_INT, 0, 0, MPI_COMM_WORLD);
    cout << "M(2,0)" << endl;

    // bonus: trimite clusterul 0 catre 1
    MPI_Send(&cluster0_len, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    cout << "M(2,1)" << endl;
    MPI_Send(&cluster0[0], cluster0_len, MPI_INT, 1, 0, MPI_COMM_WORLD);
    cout << "M(2,1)" << endl;

    // trimite fiecarui worker al sau intreaga topologie (0, 1, 2)
    for (int i = 0; i < cluster_size; i++) {
      MPI_Send(&cluster0_len, 1, MPI_INT, current_cluster[i], 0,
               MPI_COMM_WORLD);
      cout << "M(2," << current_cluster[i] << ")" << endl;
      MPI_Send(&cluster0[0], cluster0_len, MPI_INT, current_cluster[i], 0,
               MPI_COMM_WORLD);
      cout << "M(2," << current_cluster[i] << ")" << endl;

      MPI_Send(&cluster1_len, 1, MPI_INT, current_cluster[i], 0,
               MPI_COMM_WORLD);
      cout << "M(2," << current_cluster[i] << ")" << endl;
      MPI_Send(&cluster1[0], cluster1_len, MPI_INT, current_cluster[i], 0,
               MPI_COMM_WORLD);
      cout << "M(2," << current_cluster[i] << ")" << endl;

      MPI_Send(&cluster_size, 1, MPI_INT, current_cluster[i], 0,
               MPI_COMM_WORLD);
      cout << "M(2," << current_cluster[i] << ")" << endl;
      MPI_Send(&current_cluster[0], cluster_size, MPI_INT, current_cluster[i],
               0, MPI_COMM_WORLD);
      cout << "M(2," << current_cluster[i] << ")" << endl;
    }

  } else if (rank == 0) {
    // primeste clusterul2 de la 2
    int cluster2_len;
    MPI_Recv(&cluster2_len, 1, MPI_INT, 2, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    vector<int> cluster2(cluster2_len, 0);
    MPI_Recv(&cluster2[0], cluster2_len, MPI_INT, 2, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    graph[2] = cluster2;

    vector<int> current_cluster = graph[rank];
    int cluster_size = current_cluster.size();
    // trimite clusterul sau catre 2
    MPI_Send(&cluster_size, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
    cout << "M(0,2)" << endl;
    MPI_Send(&current_cluster[0], cluster_size, MPI_INT, 2, 0, MPI_COMM_WORLD);
    cout << "M(0,2)" << endl;

    // primeste clusterul1 de la 2
    int cluster1_len;
    MPI_Recv(&cluster1_len, 1, MPI_INT, 2, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    vector<int> cluster1(cluster1_len, 0);
    MPI_Recv(&cluster1[0], cluster1_len, MPI_INT, 2, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    graph[1] = cluster1;

    // trimite fiecarui worker al sau intreaga topologie (0, 1, 2)
    for (int i = 0; i < cluster_size; i++) {
      MPI_Send(&cluster_size, 1, MPI_INT, current_cluster[i], 0,
               MPI_COMM_WORLD);
      cout << "M(0," << current_cluster[i] << ")" << endl;
      MPI_Send(&current_cluster[0], cluster_size, MPI_INT, current_cluster[i],
               0, MPI_COMM_WORLD);
      cout << "M(0," << current_cluster[i] << ")" << endl;

      MPI_Send(&cluster1_len, 1, MPI_INT, current_cluster[i], 0,
               MPI_COMM_WORLD);
      cout << "M(0," << current_cluster[i] << ")" << endl;
      MPI_Send(&cluster1[0], cluster1_len, MPI_INT, current_cluster[i], 0,
               MPI_COMM_WORLD);
      cout << "M(0," << current_cluster[i] << ")" << endl;

      MPI_Send(&cluster2_len, 1, MPI_INT, current_cluster[i], 0,
               MPI_COMM_WORLD);
      cout << "M(0," << current_cluster[i] << ")" << endl;
      MPI_Send(&cluster2[0], cluster2_len, MPI_INT, current_cluster[i], 0,
               MPI_COMM_WORLD);
      cout << "M(0," << current_cluster[i] << ")" << endl;
    }
  } else if (rank == 1) {
    // primeste clusterul2 de la 2
    int cluster2_len;
    MPI_Recv(&cluster2_len, 1, MPI_INT, 2, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    vector<int> cluster2(cluster2_len, 0);
    MPI_Recv(&cluster2[0], cluster2_len, MPI_INT, 2, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    graph[2] = cluster2;

    vector<int> current_cluster = graph[rank];
    int cluster_size = current_cluster.size();
    // trimite clusterul sau catre 2
    MPI_Send(&cluster_size, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
    cout << "M(1,2)" << endl;
    MPI_Send(&current_cluster[0], cluster_size, MPI_INT, 2, 0, MPI_COMM_WORLD);
    cout << "M(1,2)" << endl;

    // primeste clusterul0 de la 2
    int cluster0_len;
    MPI_Recv(&cluster0_len, 1, MPI_INT, 2, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    vector<int> cluster0(cluster0_len, 0);
    MPI_Recv(&cluster0[0], cluster0_len, MPI_INT, 2, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    graph[0] = cluster0;

    // trimite fiecarui worker al sau intreaga topologie (0, 1, 2)
    for (int i = 0; i < cluster_size; i++) {
      MPI_Send(&cluster0_len, 1, MPI_INT, current_cluster[i], 0,
               MPI_COMM_WORLD);
      cout << "M(1," << current_cluster[i] << ")" << endl;
      MPI_Send(&cluster0[0], cluster0_len, MPI_INT, current_cluster[i], 0,
               MPI_COMM_WORLD);
      cout << "M(1," << current_cluster[i] << ")" << endl;

      MPI_Send(&cluster_size, 1, MPI_INT, current_cluster[i], 0,
               MPI_COMM_WORLD);
      cout << "M(1," << current_cluster[i] << ")" << endl;
      MPI_Send(&current_cluster[0], cluster_size, MPI_INT, current_cluster[i],
               0, MPI_COMM_WORLD);
      cout << "M(1," << current_cluster[i] << ")" << endl;

      MPI_Send(&cluster2_len, 1, MPI_INT, current_cluster[i], 0,
               MPI_COMM_WORLD);
      cout << "M(1," << current_cluster[i] << ")" << endl;
      MPI_Send(&cluster2[0], cluster2_len, MPI_INT, current_cluster[i], 0,
               MPI_COMM_WORLD);
      cout << "M(1," << current_cluster[i] << ")" << endl;
    }
  } else {
    // suntem intr-un worker
    // primim clusterele 0, 1, 2 (in aceasta ordine)
    int cluster0_len;
    MPI_Recv(&cluster0_len, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    vector<int> cluster0(cluster0_len, 0);
    MPI_Recv(&cluster0[0], cluster0_len, MPI_INT, MPI_ANY_SOURCE, 0,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    graph[0] = cluster0;

    int cluster1_len;
    MPI_Recv(&cluster1_len, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    vector<int> cluster1(cluster1_len, 0);
    MPI_Recv(&cluster1[0], cluster1_len, MPI_INT, MPI_ANY_SOURCE, 0,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    graph[1] = cluster1;

    int cluster2_len;
    MPI_Recv(&cluster2_len, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    vector<int> cluster2(cluster2_len, 0);
    MPI_Recv(&cluster2[0], cluster2_len, MPI_INT, MPI_ANY_SOURCE, 0,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    graph[2] = cluster2;
  }

  // afisare topologie
  cout << rank << " -> ";
  for (int i = 0; i < 3; i++) {
    cout << i << ":";
    for (int j = 0; j < graph[i].size(); j++) {
      cout << graph[i][j];
      if (j != graph[i].size() - 1) {
        cout << ",";
      }
    }
    cout << " ";
  }
  cout << endl;

  if (rank == 0) {
    vector<int> v;
    int length = atoi(argv[1]);
    for (int i = 0; i < length; i++) {
      v.push_back(i);
    }
    int count_workers = graph[0].size() + graph[1].size() + graph[2].size();
    int elements_per_worker = length / count_workers;
    int cluster0_el = graph[0].size() * elements_per_worker;
    int cluster1_el = graph[1].size() * elements_per_worker;
    int cluster2_el = length - cluster0_el - cluster1_el;

    // impartire vector initial pe clustere
    vector<int> cluster0_vector{v.begin(), v.begin() + cluster0_el};
    vector<int> cluster1_vector{v.begin() + cluster0_el,
                                v.begin() + cluster0_el + cluster1_el};
    vector<int> cluster2_vector{v.begin() + cluster0_el + cluster1_el, v.end()};

    // trimite catre 2 pentru a ajunge la 1
    MPI_Send(&cluster1_el, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
    cout << "M(0,2)" << endl;
    MPI_Send(&cluster1_vector[0], cluster1_el, MPI_INT, 2, 0, MPI_COMM_WORLD);
    cout << "M(0,2)" << endl;

    // trimite catre 2
    MPI_Send(&cluster2_el, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
    cout << "M(0,2)" << endl;
    MPI_Send(&cluster2_vector[0], cluster2_el, MPI_INT, 2, 0, MPI_COMM_WORLD);
    cout << "M(0,2)" << endl;

    // procesare raspuns pentru clusterul 0
    vector<int> answer_cluster0;
    for (int i = 0; i < graph[rank].size(); i++) {
      int start = i * cluster0_el / graph[rank].size();
      int end = min((i + 1) * cluster0_el / graph[rank].size(), cluster0_el);

      // impartire pe workeri
      vector<int> worker_elements;
      for (int j = start; j < end; j++) {
        worker_elements.push_back(cluster0_vector[j]);
      }
      int size = worker_elements.size();

      MPI_Send(&size, 1, MPI_INT, graph[rank][i], 0, MPI_COMM_WORLD);
      cout << "M(0," << graph[rank][i] << ")" << endl;
      MPI_Send(&worker_elements[0], size, MPI_INT, graph[rank][i], 0,
               MPI_COMM_WORLD);
      cout << "M(0," << graph[rank][i] << ")" << endl;

      int answer_worker_len;
      MPI_Recv(&answer_worker_len, 1, MPI_INT, graph[rank][i], 0,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      vector<int> answer_worker(answer_worker_len, 0);
      MPI_Recv(&answer_worker[0], answer_worker_len, MPI_INT, graph[rank][i], 0,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      for (auto el : answer_worker) {
        answer_cluster0.push_back(el);
      }
    }

    // preluare raspunsuri de la celelalte clustere
    int answer_cluster1_len;
    MPI_Recv(&answer_cluster1_len, 1, MPI_INT, 2, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    vector<int> answer_cluster1(answer_cluster1_len, 0);
    MPI_Recv(&answer_cluster1[0], answer_cluster1_len, MPI_INT, 2, 0,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int answer_cluster2_len;
    MPI_Recv(&answer_cluster2_len, 1, MPI_INT, 2, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    vector<int> answer_cluster2(answer_cluster2_len, 0);
    MPI_Recv(&answer_cluster2[0], answer_cluster2_len, MPI_INT, 2, 0,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    vector<int> answer;
    for (auto el : answer_cluster0) {
      answer.push_back(el);
    }
    for (auto el : answer_cluster1) {
      answer.push_back(el);
    }
    for (auto el : answer_cluster2) {
      answer.push_back(el);
    }
    cout << "Rezultat: ";
    for (auto el : answer) {
      cout << el << " ";
    }
    cout << endl;
  } else if (rank == 2) {
    // bonus: legatura dintre 0 si 1 se face prin 2
    int cluster1_len;
    MPI_Recv(&cluster1_len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    vector<int> cluster1(cluster1_len, 0);
    MPI_Recv(&cluster1[0], cluster1_len, MPI_INT, 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    MPI_Send(&cluster1_len, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    cout << "M(2,1)" << endl;
    MPI_Send(&cluster1[0], cluster1_len, MPI_INT, 1, 0, MPI_COMM_WORLD);
    cout << "M(2,1)" << endl;

    int answer_cluster1_len;
    MPI_Recv(&answer_cluster1_len, 1, MPI_INT, 1, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    vector<int> answer_cluster1(answer_cluster1_len, 0);
    MPI_Recv(&answer_cluster1[0], answer_cluster1_len, MPI_INT, 1, 0,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    MPI_Send(&answer_cluster1_len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    cout << "M(2,0)" << endl;
    MPI_Send(&answer_cluster1[0], answer_cluster1_len, MPI_INT, 0, 0,
             MPI_COMM_WORLD);
    cout << "M(2,0)" << endl;

    // primesc datele pentru clusterul 2
    int cluster2_len;
    MPI_Recv(&cluster2_len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    vector<int> cluster2(cluster2_len, 0);
    MPI_Recv(&cluster2[0], cluster2_len, MPI_INT, 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    vector<int> answer_cluster2;
    for (int i = 0; i < graph[rank].size(); i++) {
      int start = i * cluster2_len / graph[rank].size();
      int end = min((i + 1) * cluster2_len / graph[rank].size(), cluster2_len);

      vector<int> worker_elements;
      for (int j = start; j < end; j++) {
        worker_elements.push_back(cluster2[j]);
      }
      int size = worker_elements.size();

      MPI_Send(&size, 1, MPI_INT, graph[rank][i], 0, MPI_COMM_WORLD);
      cout << "M(2," << graph[rank][i] << ")" << endl;
      MPI_Send(&worker_elements[0], size, MPI_INT, graph[rank][i], 0,
               MPI_COMM_WORLD);
      cout << "M(2," << graph[rank][i] << ")" << endl;

      int answer_worker_len;
      MPI_Recv(&answer_worker_len, 1, MPI_INT, graph[rank][i], 0,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      vector<int> answer_worker(answer_worker_len, 0);
      MPI_Recv(&answer_worker[0], answer_worker_len, MPI_INT, graph[rank][i], 0,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      for (auto el : answer_worker) {
        answer_cluster2.push_back(el);
      }
    }

    // trimitere date spre clusterul 0 pentru procesarea raspunsului
    int answer_cluster2_len = answer_cluster2.size();
    MPI_Send(&answer_cluster2_len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    cout << "M(2,0)" << endl;
    MPI_Send(&answer_cluster2[0], answer_cluster2_len, MPI_INT, 0, 0,
             MPI_COMM_WORLD);
    cout << "M(2,0)" << endl;
  } else if (rank == 1) {
    int cluster1_len;
    MPI_Recv(&cluster1_len, 1, MPI_INT, 2, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    vector<int> cluster1(cluster1_len, 0);
    MPI_Recv(&cluster1[0], cluster1_len, MPI_INT, 2, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    vector<int> answer_cluster1;
    for (int i = 0; i < graph[rank].size(); i++) {
      int start = i * cluster1_len / graph[rank].size();
      int end = min((i + 1) * cluster1_len / graph[rank].size(), cluster1_len);

      vector<int> worker_elements;
      for (int j = start; j < end; j++) {
        worker_elements.push_back(cluster1[j]);
      }
      int size = worker_elements.size();

      MPI_Send(&size, 1, MPI_INT, graph[rank][i], 0, MPI_COMM_WORLD);
      cout << "M(1," << graph[rank][i] << ")" << endl;
      MPI_Send(&worker_elements[0], size, MPI_INT, graph[rank][i], 0,
               MPI_COMM_WORLD);
      cout << "M(1," << graph[rank][i] << ")" << endl;

      int answer_worker_len;
      MPI_Recv(&answer_worker_len, 1, MPI_INT, graph[rank][i], 0,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      vector<int> answer_worker(answer_worker_len, 0);
      MPI_Recv(&answer_worker[0], answer_worker_len, MPI_INT, graph[rank][i], 0,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      for (auto el : answer_worker) {
        answer_cluster1.push_back(el);
      }
    }

    // trimitere subvector modificat inapoi la 2
    int answer_cluster1_len = answer_cluster1.size();
    MPI_Send(&answer_cluster1_len, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
    cout << "M(1,2)" << endl;
    MPI_Send(&answer_cluster1[0], answer_cluster1_len, MPI_INT, 2, 0,
             MPI_COMM_WORLD);
    cout << "M(1,2)" << endl;

  } else {
    // workerul prelucreaza ceea ce primeste si trimite inapoi
    int cluster = get_cluster(rank, graph);
    if (cluster == -1) {
      cout << "ERROR\n";
      return -1;
    }
    int worker_len;
    MPI_Recv(&worker_len, 1, MPI_INT, cluster, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    vector<int> answer_worker(worker_len, 0);
    MPI_Recv(&answer_worker[0], worker_len, MPI_INT, cluster, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    for (auto &el : answer_worker) {
      el = el * 2;
    }

    MPI_Send(&worker_len, 1, MPI_INT, cluster, 0, MPI_COMM_WORLD);
    cout << "M(" << rank << "," << cluster << ")\n";
    MPI_Send(&answer_worker[0], worker_len, MPI_INT, cluster, 0,
             MPI_COMM_WORLD);
    cout << "M(" << rank << "," << cluster << ")\n";
  }
  MPI_Finalize();
  return 0;
}