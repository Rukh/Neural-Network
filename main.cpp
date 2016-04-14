#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>

using namespace std;

struct Neuron {
    int number_of_links;
    float links_weight[5][60];
    float synapse_weight;
};

class DataCell {
public:
    float param1;
    float param2;
    float param3;
    float param4;
    int param5;

    void show() {
        cout << param1 << ' '
             << param2 << ' '
             << param3 << ' '
             << param4 << ' '
             << param5 << '\n';
    }
};

class InputDataStream {
    int data_size = 43200;
    int current = 0;
    DataCell data[43200];
public:
    InputDataStream () {
        ifstream fin;
        fin.open("EURUSDpro1.txt");
        for(int i = 0; i < data_size; i++) {
            if(fin.eof()) {
                data_size = i;
                break;
            }
            fin >> data[i].param1;
            fin >> data[i].param2;
            fin >> data[i].param3;
            fin >> data[i].param4;
            fin >> data[i].param5;
            if(i != 0) {
                data[i - 1].param1 -= data[i].param1;
                data[i - 1].param2 -= data[i].param2;
                data[i - 1].param3 -= data[i].param3;
                data[i - 1].param4 -= data[i].param4;
                data[i - 1].param5 -= data[i].param5;
            }
        }
    }

    bool eof() {
        if(current == data_size) return true;
        else return false;
    }

    DataCell GetTick() {
        DataCell temp = data[current];
        current++;
        return temp;
    }

    void show() {
        for(int i = 0; i < data_size; i++) {
            data[i].show();
        }
    }
};

class PrimaryNeuronalLayer {
public:
    int number_of_neurons;
    Neuron* neuronal_layer;

    void RandomFilling() {
        number_of_neurons = rand() % 100 + 1;
        neuronal_layer = new Neuron[number_of_neurons];
        for(int i = 0; i < number_of_neurons; i++) {
            neuronal_layer[i].number_of_links = rand() % 100 + 1;
            //neuronal_layer[i].links_weight = new float[neuronal_layer[i].number_of_links];
            for(int j = 0; j < neuronal_layer[i].number_of_links; j++) {
                //neuronal_layer[i].links_weight[j] = float(rand()) / RAND_MAX;
            }
            neuronal_layer[i].synapse_weight = float(rand()) / RAND_MAX;
        }
    }

    void Load(char* file_path) {
        ifstream fin;
        fin.open(file_path);

        fin >> number_of_neurons;
        neuronal_layer = new Neuron[number_of_neurons];
        for(int i = 0; i < number_of_neurons; i++) {
            fin >> neuronal_layer[i].number_of_links;
            //neuronal_layer[i].links_weight = new float[neuronal_layer[i].number_of_links];
            for(int j = 0; j < neuronal_layer[i].number_of_links; j++) {
                //fin >> neuronal_layer[i].links_weight[j];
            }
            fin >> neuronal_layer[i].synapse_weight;
        }
    }

    void Save(char* file_path) {
        ofstream fout;
        fout.open(file_path);
        fout.clear();

        fout << number_of_neurons << '\n';
        for(int i = 0; i < number_of_neurons; i++) {
            fout << neuronal_layer[i].number_of_links << '\n';
            for(int j = 0; j < neuronal_layer[i].number_of_links; j++) {
                fout << neuronal_layer[i].links_weight[j] << ' ';
            }
            fout << neuronal_layer[i].synapse_weight << '\n';
        }
        fout.close();
    }

    PrimaryNeuronalLayer() {
        RandomFilling();
        Save("PrimaryNeuronalLayer.txt");

    }
};

int main()
{
    cout << "Hello world!" << endl;
    return 0;
}
