# Сортировка пузырьком (алгоритм чет-нечетной перестановки)

- Студент: <Краснопевцева Вероника Дмитриевна>, группа 3823Б1ПМоп3
- Технологии: SEQ | MPI 
- Вариант: 21

## 1. Введение

Сортировка является одной из фундаментальных задач в програмировании. Среди множества алгоритмов сортировки особое место занимает пузырьковая сортировка (bubble sort) благодаря своей простоте и наглядности. Однако её стандартная реализация имеет квадратичную сложность O(n^2), что делает её неэффективной для больших массивов.  

Алгоритм чет-нечетной перестановки представляет собой модификацию пузырьковой сортировки, которая хорошо подходит для реализации на многопроцессорных системах. В отличие от классической пузырьковой сортировки, он позволяет выполнять сравнения элементов независимо в чередующихся фазах, что делает его идеальным кандидатом для параллелизации.  

Цель работы — реализация последовательной и параллельной версий алгоритма чет-нечетной сортировки с использованием MPI и сравнение их эффективности.  

## 2. Постановка задачи

### Задача: реализация последовательной (SEQ) и параллельной (MPI) версий алгоритма чет-нечетной перестановки для сортировки целочисленных массивов.

### Формат входных данных:
- Вектор из элементов типа int

### Формат выходных данных:
- Отсортированный вектор из элементов типа int

### Ограничения:
- Массив не должен быть пустым

## 3. Описание алгоритма
### Базовый алгоритм чет-нечетной перестановки:

1. Инициализация флага отсортированности sorted = false
2. Пока массив не отсортирован (!sorted):
3. Установить sorted = true
4. Нечетная фаза: сравнить пары элементов с нечетными индексами 
5. Четная фаза: сравнить пары элементов с четными индексами 
6. Если в какой-либо фазе были перестановки, установить sorted = false
7. Возврат отсортированного массива

## 4. Схема распараллеливания

### Распределение данных

Массив делится на примерно равные части между процессами.  
Базовый размер блока: chunk_size = global_size / proc_count  
Остаток: remainder = global_size % proc_count  
Процесс с рангом rank получает: local_size = chunk_size + (rank < remainder ? 1 : 0)  

Для p процессов выполняется p итераций:  

- Четные итерации: процессы с четными рангами обмениваются с правыми соседями
- Нечетные итерации: процессы с нечетными рангами обмениваются с правыми соседями

### Локальная сортировка 
Каждый процесс сортирует свою часть с помощью seq-реализации.

### Обмен данными между процессами

- Обмен размерами локальных массивов (MPI_Sendrecv)

- Обмен самими данными (MPI_Sendrecv)

- Слияние двух отсортированных массивов (std::ranges::merge)

- Каждый процесс оставляет нужную часть (меньшую или большую)

## 5. Детали реализации 

Ключевые классы и функции: 
- класс KrasnopevtsevaVBubbleSortMPI с реализацией параллельной версии алгоритма
- класс KrasnopevtsevaVBubbleSortSEQ с реализацией последовательной версии алгоритма
- определение пространства имен namespace krasnopevtseva_v_bubble_sort с определеним используемых типов данных:  
    namespace krasnopevtseva_v_bubble_sort {
    using InType = std::vector<int>;        // Входной массив
    using OutType = std::vector<int>;       // Выходной (отсортированный) массив
    using TestType = std::tuple<std::vector <int>, std::string>;  // Тестовые данные
    using BaseTask = ppc::task::Task<InType, OutType>;
}

Основные методы класса KrasnopevtsevaVBubbleSortMPI:  

- RunImpl() — основная точка входа для MPI версии, управляет всем процессом сортировки
- ParallelSort() — выполняет параллельную сортировку локальных данных
- FindPartner() — определяет партнера для обмена на текущей фазе
- DistributeData() — распределение данных между процессами
- MergeProc() — попарное слияние данных между процессами
- GatherData() — сбор результатов на всех процессах
- SeqSort() — последовательная чет-нечетная сортировка

## 6. Окружение
- Windows: AMD Ryzen 7 5700X 8-Core Processor, 32.0 ГБ ОП, Windows 11 Pro 25H2
- Набор инструментов: DevContainer:compiler gcc (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0, Open MPI 4.1.6, build type Release

## 7. Результаты

### 7.1 Корректность

Проверка корректности алгоритма осуществляется через цикл, который проверяет условие (output_data[i] > output_data[i + 1]) для всех элементов массива, и если где-то это условие истинно, то массив не отсортирован.

### 7.2 Производительность


| Mode        | Count | Time,s  | Speedup | Efficiency |
|-------------|-------|---------|---------|------------|
| seq         | 1     | 36.143  | 1.00    | N/A        |
| mpi         | 2     | 6.812   | 5.3     | 265.0%     |
| mpi         | 4     | 1.305   | 27.7    | 692.5%     |

Расчеты:  
- Speedup = T_seq / T_parallel  
- Для 2 процессов: 36.143 / 6.812 = 5.3
- Для 4 процессов: 36.143 / 1.305 = 27.7

- Efficiency = Speedup / Count × 100%
- Для 2 процессов: 5.3 / 2 × 100% = 265.0%
- Для 4 процессов: 27.7 / 4 × 100% = 692.5%

Проверка производительности осуществлялась на массиве длины 100000 со случайными числами.

## 8. Заключение

Вывод: Реализация алгоритма чет-нечетной перестановки демонстрирует отличную масштабируемость и может эффективно использоваться на многопроцессорных системах. Эффективность параллельной версии зависит от размера данных - чем больше массив, тем эффективнее MPI-версия.

## 9. Приложения
```cpp
bool KrasnopevtsevaVBubbleSortMPI::RunImpl() {
  auto input = GetInput();
  int rank = 0;
  int kol = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &kol);
  if (input.size() <= static_cast<size_t>(kol) || (kol == 1)) {
    SeqSort(input);
    GetOutput() = input;
    return true;
  }

  size_t global_size = input.size();

  std::vector<int> local_data = DistributeData(input, rank, kol);

  ParallelSort(local_data, rank, kol);

  std::vector<int> result = GatherData(local_data, rank, kol, global_size);
  GetOutput() = result;

  return true;
}

void KrasnopevtsevaVBubbleSortMPI::ParallelSort(std::vector<int> &local_data, int rank, int kol) {
  if (local_data.empty() || kol <= 1) {
    return;
  }

  SeqSort(local_data);

  for (int phase = 0; phase < kol; phase++) {
    int partner = FindPartner(rank, kol, phase);

    if (partner != -1) {
      bool keep_smaller = (rank < partner);  
      MergeProc(local_data, partner, keep_smaller);
    }

    MPI_Barrier(MPI_COMM_WORLD);
  }
}

int KrasnopevtsevaVBubbleSortMPI::FindPartner(int rank, int kol, int phase) {
  const bool same_parity = ((phase % 2) == (rank % 2));

  if (same_parity && rank + 1 < kol) {
    return rank + 1;
  }
  if (!same_parity && rank > 0) {
    return rank - 1;
  }
  return -1;
}

std::vector<int> KrasnopevtsevaVBubbleSortMPI::DistributeData(const std::vector<int> &input, int rank, int kol) {
  int global_size = static_cast<int>(input.size());
  MPI_Bcast(&global_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  int base_chunk = global_size / kol;
  int remainder = global_size % kol;

  std::vector<int> send_counts(kol, base_chunk);
  std::vector<int> displacements(kol, 0);

  for (int i = 0; i < kol; i++) {
    if (i < remainder) {
      send_counts[i]++;
    }
    if (i > 0) {
      displacements[i] = displacements[i - 1] + send_counts[i - 1];
    }
  }

  int local_size = send_counts[rank];
  std::vector<int> local_data(local_size);

  MPI_Scatterv(rank == 0 ? input.data() : nullptr, send_counts.data(), displacements.data(), MPI_INT, local_data.data(),
               local_size, MPI_INT, 0, MPI_COMM_WORLD);

  return local_data;
}

void KrasnopevtsevaVBubbleSortMPI::MergeProc(std::vector<int> &data, int partner_rank, bool keep_smaller) {
  int my_size = static_cast<int>(data.size());
  int partner_size = 0;

  MPI_Sendrecv(&my_size, 1, MPI_INT, partner_rank, 0, &partner_size, 1, MPI_INT, partner_rank, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);

  if (partner_size == 0 || my_size == 0) {
    return;
  }

  std::vector<int> partner_data(partner_size);
  MPI_Sendrecv(data.data(), my_size, MPI_INT, partner_rank, 1, partner_data.data(), partner_size, MPI_INT, partner_rank,
               1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  std::vector<int> merged(my_size + partner_size);
  std::ranges::merge(data.begin(), data.end(), partner_data.begin(), partner_data.end(), merged.begin());

  if (keep_smaller) {
    std::ranges::copy(merged.begin(), merged.begin() + my_size, data.begin());
  } else {
    std::ranges::copy(merged.end() - my_size, merged.end(), data.begin());
  }
}

std::vector<int> KrasnopevtsevaVBubbleSortMPI::GatherData(const std::vector<int> &local_data, int rank, int kol,
                                                          size_t global_size) {
  std::vector<int> result;

  if (rank == 0) {
    result.resize(global_size);
    size_t offset = 0;

    int my_size = static_cast<int>(local_data.size());
    if (my_size > 0) {
      std::ranges::copy(local_data.begin(), local_data.end(), result.begin() + static_cast<ptrdiff_t>(offset));
      offset += my_size;
    }

    for (int i = 1; i < kol; i++) {
      int remote_size = 0;
      MPI_Recv(&remote_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      if (remote_size > 0) {
        MPI_Recv(result.data() + offset, remote_size, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        offset += remote_size;
      }
    }
  } else {
    int local_size = static_cast<int>(local_data.size());
    MPI_Send(&local_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

    if (local_size > 0) {
      MPI_Send(local_data.data(), local_size, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }
  }

  int result_size = 0;
  if (rank == 0) {
    result_size = static_cast<int>(global_size);
  }

  MPI_Bcast(&result_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    result.resize(result_size);
  }

  MPI_Bcast(result.data(), result_size, MPI_INT, 0, MPI_COMM_WORLD);

  return result;
}

void KrasnopevtsevaVBubbleSortMPI::SeqSort(std::vector<int> &data) {
  if (data.size() <= 1) {
    return;
  }

  size_t n = data.size();
  bool sorted = false;

  while (!sorted) {
    sorted = true;
    for (size_t i = 1; i < n - 1; i += 2) {
      if (data[i] > data[i + 1]) {
        std::swap(data[i], data[i + 1]);
        sorted = false;
      }
    }
    for (size_t i = 0; i < n - 1; i += 2) {
      if (data[i] > data[i + 1]) {
        std::swap(data[i], data[i + 1]);
        sorted = false;
      }
    }
  }
}
```