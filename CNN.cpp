#include <iostream>
#include <fstream>
#include <cmath>

float error;

class BaseLayer {
    int size;
    float* array;

public:
    BaseLayer(int size) {
        array = new float[size];
        this->size = size;
        for(int i = 0; i < size; i++) {
            array[i] = 0.0;
        }
    }

    float& operator[](int index) {
        if(index >= size || index < 0) {
            throw "class: BaseLayer; function: operator[]; error: Bad index";
            return error;
        }
        else return array[index];
    }

    int length() const {
        return size;
    }

    bool loadData(std::string file_path, int index = 0) {//Загружает кусок file_path размера size из позиции index
        std::ifstream fin;
        fin.open(file_path.data());

        float temp;
        for(int i = 0; i < index; i++) {
            fin >> temp;
        }
        fin >> temp;

        array[0] = int(temp * 100000.0);
        for(int i = 1; i < size; i++) {
            fin >> temp;
            array[i] = int(temp * 100000.0);
            array[i - 1] -=  array[i];
        }
        fin >> temp;
        array[size - 1] -= int(temp * 100000.0);
        
        /*
        for(int i = 0; i < size; i++) {
            std::cout << i << ": "<< array[i] << '\n';
        }*/
        
        return true;
    }
};

class Kernel { //ядро свертки
    int size; 
    float* array;

public:
    Kernel(int size) {
        array = new float[size];
        this->size = size;
        for(int i = 0; i < size; i++) {
            array[i] = 0.0;
        }
    }
    Kernel(int size, float* mass) { //создание ядра из последовательности
        array = new float[size];
        this->size = size;
        for(int i = 0; i < size; i++) {
            array[i] = mass[i];
        }
    }
    Kernel& operator=(Kernel* var) {
        if(this->length() != var->length()) {
            throw "class: Kernel; function: operator=; error: Assignment is impossible";
        }
        for(int i = 0; i < size; i++) {
            array[i] = (*var)[i];
        }
        return *this;
    }
    float compare_with(float* mass) {// сравнение с массивом
        float deviation = 0;
        for(int i = 0; i < size; i++) {
            deviation += (array[i] - mass[i])*(array[i] - mass[i]);
        }
        deviation /= size;
        return sqrt(deviation);
    }
    float compare_with(Kernel* var) {// сравнение с другим ядром
        if(this->length() != var->length()) {
            throw "class: Kernel; function: compare_with(); error: Compare is impossible";
        }
        float deviation = 0;
        for(int i = 0; i < size; i++) {
            deviation += (array[i] - (*var)[i])*(array[i] - (*var)[i]);
        }
        deviation /= size;
        return sqrt(deviation);
    }

    float& operator[](int index) {
        if(index >= size || index < 0) {
            throw "class: Kernel; function: operator[]; error: Bad index";
            return error;
        }
        else return array[index];
    }

    int length() const {
        return size;
    }
    bool is_empty() {
        bool empty = true;
        for(int i = 0; i < size; i++) {
            if(array[i] != 0.0) empty = false;
        }
        return empty;
    }
    void show() {
        for(int i = 0; i < size; i++) {
            std::cout << array[i] << ' ';
        }
    }
};


class CNN { //Convolutional Neural Network - одномерная вариация
    int layer_size;
    int number_of_layers; //Сейчас не используется
    int kernel_size; 
    float similarity_factor;
    std::string primary_kernels_path;

    BaseLayer* base_layer;
    Kernel** kernel_array;
    int number_of_kernels;

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
        base_layer = new BaseLayer(layer_size);
        base_layer->loadData(file_path, 0);

        /*
        for(int i = 0; i < layer_size; i++) {
            std::cout << i << ": " << layer_array->[0] << '\n';
        }*/
        
        return true;
    }
    bool generationPrimaryKernels() {  //Генерация сверточных ядер первого слоя, и запись их в файл
        std::ofstream fout;
        fout.open(primary_kernels_path.data());
        
        for(similarity_factor = 1; similarity_factor < 101; similarity_factor++){

        number_of_kernels = 0;
        int feature_map_size = layer_size - (kernel_size - 1);
        kernel_array = new Kernel*[feature_map_size]; //максимальное число ядер которое может быть сгенерировано в данном алгоритме
        for (int i = 0; i < feature_map_size; i++)
        {
            kernel_array[i] = NULL;
        }
        for (int i = 0; i < feature_map_size; i++)  //первичная генерация ядер свертки
        {
            bool found_proper = false;
            for (int j = 0; j < feature_map_size; j++) //пребор ядер и проверка степени их отклонения от выборки
            {
                if(kernel_array[j] == NULL) break;
                if(kernel_array[j]->is_empty()) break;
                if(kernel_array[j]->compare_with(&((*base_layer)[i])) < similarity_factor) {
                    found_proper = true;
                    break;
                }
            }

            if(found_proper == false) { //создаем нове ядро
                kernel_array[number_of_kernels] = new Kernel(kernel_size, &((*base_layer)[i]));
                number_of_kernels++;
            }
        }

        /*
        float deviation = 0;
        bool is_coating = true; //есть ли подобие?
        int counter = 0;
        
        int** coating_buffer;
        coating_buffer = new int*[feature_map_size];
        for (int i = 0; i < feature_map_size - 1; i++)
        {
        	coating_buffer[i] = new int[feature_map_size - 1];
        }

        for (int i = 0; i < feature_map_size - 1; i++)  //поиск подобных ядер
        {
            for (int j = i + 1; j < feature_map_size - 1; ++j)
            {
                is_coating = true;
                deviation = 0;
                for (int k = 0; k < kernel_size; k++)
                {
                    deviation = std::abs(kernels_arrays[0][i]->get(k) - kernels_arrays[0][j]->get(k));
                    if(deviation > kernel_similarity_factor) is_coating = false;
                }
                if(is_coating) { //ядра подобны
                	coating_buffer[i][j] = 1;
                }
                else coating_buffer[i][j] = 0;
            }
        }
        ConvolutionKernel* temp_kernel = new ConvolutionKernel(kernel_size);
        counter = 1;
        for (int i = 0; i < feature_map_size - 1; i++)
        {
        	counter = 1;
	        for (int k = 0; k < kernel_size; k++)
            {
            	temp_kernel->set(k, kernels_arrays[0][i]->get(k));
            }
        	for (int j = i + 1; j < feature_map_size - 1; ++i)
        	{
        		if(coating_buffer[i][j] == 1){
        			for (int k = 0; k < kernel_size; k++)
                    {
                    	temp_kernel->set(k, temp_kernel->get(k) + kernels_arrays[0][j]->get(k));
                    	counter++;
                        
                    }
        		}
        	}
        	for (int k = 0; k < kernel_size; k++)
            {
            	kernels_arrays[0][i]->set(k, temp_kernel->get(k) / counter);
            }
        }
        float temp = 0;
        */

        /*
        std::cout << '\n';
        for (int i = 0; i < number_of_kernels; i++)  //вывод на экран
        {
            std::cout << "kernel #" << i << ": ";
            if(kernel_array[i] != NULL) kernel_array[i]->show();
            std::cout << '\n';
        }*/
        std::cout << "Amount of kernels: (" << number_of_kernels << " / " << feature_map_size << ")\n";

        /*
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
        */
        /*
        counter = 0;
        for (int i = 0; i < feature_map_size; i++)  //расчитаем степень поккрытия ядрами свертки исходного слоя
        {
            //std::cout << i << ": ";
            for (int j = 0; j < feature_map_size; j++)  //Пробегаем по все ядрам
            {
            	is_coating = true;
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
        */
        fout << number_of_kernels << '\n';
        }
        return true;
    }
};

int main() {
    /*
    UCNN Mind;
    char ch;
    for (;;)
    {
        std::cout << "Load settings ------> " << Mind.loadSettings("/Users/Dmitry/Desktop/Neural-Network/settings.txt") << '\n';
        std::cout << "Initialize ---------> " << Mind.initialize() << '\n';
        std::cout << "Load data ----------> " << Mind.loadData("/Users/Dmitry/Desktop/Neural-Network/data.txt") << '\n';
        std::cout << "Generation kernels -> " << Mind.generationPrimaryKernels() << '\n';
        std::cout << "Do you want exit?: "; std::cin >> ch;
        if (ch == 'y') return 0;
    }*/
    CNN Mind;
    char ch;
    for (;;)
    {
        std::cout << "Load settings ------> " << Mind.loadSettings("/Users/Dmitry/Desktop/Neural-Network/settings.txt") << '\n';
        std::cout << "Load data ----------> " << Mind.loadData("/Users/Dmitry/Desktop/Neural-Network/data.txt") << '\n';
        std::cout << "Generation kernels -> " << Mind.generationPrimaryKernels() << '\n';
        std::cout << "Do you want exit?: "; std::cin >> ch;
        if (ch == 'y') return 0;
    }
    return 0;
}
