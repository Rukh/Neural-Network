#include <iostream>
#include <fstream>
#include <cmath>

class Layer {
    int size;
    int* array;

public:
    Layer(int size) {
        array = new int[size];
        this->size = size;
        for(int i = 0; i < size; i++) {
            array[i] = 0.0;
        }
    }
    int get(int i) {
        return array[i];
    }
    bool loadData(std::string file_path, int index) {//Загружает кусок file_path размера size из позиции index
        std::ifstream fin;
        fin.open(file_path.data());
        float temp;
        for(int i = 0; i < index; i++) {
            fin >> temp;
        }
        fin >> temp;
        array[0] = temp * 100000.0;
        for(int i = 1; i < size; i++) {
            fin >> temp;
            array[i] = temp * 100000.0;
            array[i - 1] -=  array[i];
        }
        fin >> temp;
        array[size - 1] -= temp * 100000.0;
        /*
        for(int i = 0; i < size; i++) {
            std::cout << array[i] << ' ';
        }
        */
    }
    void set(int i, int value) {
        array[i] = value;
    }
};

class ConvolutionKernel {
    int size;
    float* array;

public:
    ConvolutionKernel(int size) {
        array = new float[size];
        this->size = size;
        for(int i = 0; i < size; i++) {
            array[i] = 0.0;
        }
    }
    float get(int i) {
        return array[i];
    }
    void set(int i, float value) {
        array[i] = value;
    }
};

class UCNN { //Univariate Convolutional Neural Network
    int layer_size;
    int number_of_layers;
    int kernel_size;
    float similarity_factor;
    std::string primary_kernels_path;

    Layer** layer_array;                    //указатели на входные слои данных
    ConvolutionKernel*** kernels_arrays;       //набор весовых матриц для каждого слоя

public:
    bool loadSettings(std::string file_path = "settings.txt") {
        std::ifstream fin;
        fin.open(file_path.data());
        std::string temp;
        fin >> temp; if (temp == "layer_size:") fin >> layer_size; else return false;
        fin >> temp; if (temp == "number_of_layers:") fin >> number_of_layers; else return false;
        fin >> temp; if (temp == "kernel_size:") fin >> kernel_size; else return false;
        fin >> temp; if (temp == "similarity_factor:") fin >> similarity_factor; else return false;
        fin >> temp; if (temp == "primary_kernels_path:") fin >> primary_kernels_path; else return false;
        fin.close();
        return true;
    }
    bool loadData(std::string file_path = "data.txt") {
        std::ifstream fin;
        fin.open(file_path.data());
        layer_array[0] = new Layer(layer_size);
        layer_array[0]->loadData(file_path, 0);

        /*
        for(int i = 0; i < layer_size; i++) {
            std::cout << i << ": " << layer_array[0]->get(i) << '\n';
        }
        */
        return true;
    }
    bool initialize() {
        layer_array = new Layer*[number_of_layers];
        kernels_arrays = new ConvolutionKernel**[number_of_layers];
        return true;
    }
    float getData(int index) { //0 элемент является самым новым
        return layer_array[0]->get(index);
    }
    bool generationPrimaryKernels() {  //Генерация сверточных ядер первого слоя, и запись их в файл
        std::ofstream fout;
        fout.open(primary_kernels_path.data());
        int feature_map_size = layer_size - (kernel_size - 1);
        kernels_arrays[0] = new ConvolutionKernel*[feature_map_size]; //максимальное число ядер которое может быть сгенерировано в данном алгоритме
        for (int i = 0; i < feature_map_size; i++)  //первичная генерация ядер свертки
        {
            kernels_arrays[0][i] = new ConvolutionKernel(kernel_size);
            for (int j = 0; j < kernel_size; j++)
            {
                kernels_arrays[0][i]->set(j, layer_array[0]->get(i + j));
            }
        }

        float deviation = 0;
        bool is_coating = true; //есть ли подобие?
        int counter = 0;
        /*
        for (int i = 0; i < feature_map_size - 1; i++)  //поиск подобных ядер
        {
            for (int j = i + 1; j < feature_map_size - 1; ++j)
            {
                is_coating = true;
                deviation = 0;
                for (int k = 0; k < kernel_size; k++)
                {
                    deviation = std::abs(kernels_arrays[0][i]->get(k) - kernels_arrays[0][j]->get(k));
                    if(deviation > similarity_factor / 2) is_coating = false;
                }
                if(is_coating) { //ядра подобны
                    for (int k = 0; k < kernel_size; k++)
                    {
                        //kernels_arrays[0][i]->set(k, (kernels_arrays[0][i]->get(k) + kernels_arrays[0][j]->get(k)) / 2);
                        kernels_arrays[0][j]->set(k, 0.0);
                    }
                }
            }
        }
        */
        float temp = 0;

        
        for (int i = 0; i < feature_map_size; i++)  //вывод на экран
        {
            std::cout << "kernel #" << i << ": ";
            for (int j = 0; j < kernel_size; j++)
            {
                std::cout << kernels_arrays[0][i]->get(j) << ' ';
            }
            std::cout << '\n';
        }
        


        for (int i = 0; i < feature_map_size; i++)  //посчитаем не нулевые ядра и выведем на экран
        {
            temp = 0;
            for (int k = 0; k < kernel_size; k++)
            {
                temp += kernels_arrays[0][i]->get(k) * kernels_arrays[0][i]->get(k);
            }
            if(temp != 0.0) counter++;
        }
        std::cout << "Amount of kernels: (" << counter << " / " << feature_map_size << ")\n";

        counter = 0;
        for (int i = 0; i < feature_map_size; i++)  //расчитаем степень поккрытия ядрами свертки исходного слоя
        {
            //std::cout << i << ": ";
            is_coating = true;
            for (int j = 0; j < feature_map_size; j++)  //Пробегаем по все ядрам
            {
                temp = 0;
                for (int p = 0; p < kernel_size; p++) //Пропускаем нулевые ядра
                {
                    temp += kernels_arrays[0][j]->get(p) * kernels_arrays[0][j]->get(p);
                }
                if(temp == 0.0) continue;

                deviation = 0.0;
                
                for (int k = 0; k < kernel_size; k++) //найдем отклонение текущего ядра от выборки
                {
                    deviation = std::abs(layer_array[0]->get(i + k) - kernels_arrays[0][j]->get(k));
                    if(deviation > similarity_factor) is_coating = false;
                }
                if(is_coating) break;
            }
            if(is_coating) counter++;
            //std::cout << '\n' << '\n' << '\n';
        }
        std::cout << "Coverage area: " << (float(counter) / feature_map_size) * 100 << "%\n";
        return true;
    }
};

int main() {
    UCNN Mind;
    char ch;
    for (;;)
    {
        std::cout << "Load settings ------> " << Mind.loadSettings() << '\n';
        std::cout << "Initialize ---------> " << Mind.initialize() << '\n';
        std::cout << "Load data ----------> " << Mind.loadData() << '\n';
        std::cout << "Generation kernels -> " << Mind.generationPrimaryKernels() << '\n';
        std::cout << "Do you want exit?: "; std::cin >> ch;
        if (ch == 'y') return 0;
    }
    return 0;
}
