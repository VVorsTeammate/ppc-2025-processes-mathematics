
# Обобщённая передача от одного всем (scatter)
- Студент: Крымова Кристина Дмитриевна, 3823Б1ПМоп3
- Технология: SEQ, MPI 
- Вариант: 4

---

## 1. Введение
Коллективные операции в MPI являются фундаментальным инструментом для параллельной обработки данных. Операция `MPI_Scatter` используется для распределения частей одного буфера с корневого процесса на все остальные процессы коммуникатора.

Целью данной работы является реализация собственной функции `Scatter` на основе биномиального дерева.

---

## 2. Постановка задачи
Необходимо реализовать параллельный алгоритм, эквивалентный функции `MPI_Scatter`, со следующими требованиями:
- **Входные данные:** Исходный буфер (на root), количество отправляемых элементов, тип данных, буфер приема, количество принимаемых элементов, ранк корня и коммуникатор.
- **Выходные данные:** Каждый процесс получает часть данных, соответствующую его рангу.
- **Ограничения:**
    - Разрешено использовать только `MPI_Send` и `MPI_Recv`.
    - Поддержка типов: `MPI_INT`, `MPI_FLOAT`, `MPI_DOUBLE`.
    - Корректная обработка произвольного корневого процесса (root), отличного от 0.
    - Передача данных должна осуществляться по топологии «дерево».

---

## 3. Базовый алгоритм (Последовательный)
Последовательная версия симулирует работу `Scatter` для одного процесса или выступает в роли заглушки для проверки корректности копирования памяти. Поскольку в последовательном режиме взаимодействие между процессами отсутствует, алгоритм сводится к следующему:
1. Валидация входных размеров и типов данных.
2. Преобразование входного `void*` указателя во временный вектор байтов (`std::vector<uint8_t>`).
3. Копирование данных из исходного вектора в выходной (и в пользовательский буфер `dst_buffer`, если он задан) с использованием оператора присваивания векторов и  `std::copy`.

---

## 4. Схема распараллеливания
Для оптимизации коммуникаций используется алгоритм **Биномиального дерева**.

**Ключевые концепции:**
1.  Для поддержки произвольного `root` используется отображение физических рангов в виртуальные, где `root` становится виртуальным нулем:
    `relative_rank = (rank - root + size) % size;`
2.  Алгоритм работает шагами (`stride`), равными степеням двойки.
    - Начальный шаг: наибольшая степень двойки, меньшая `size`.
    - Цикл продолжается, пока `stride > 0`.
3.  **Роли процессов:**
    - Если `relative_rank % (2 * stride) == 0`: Процесс выступает **Отправителем**. Он отправляет данные процессу `virt_dest = relative_rank + stride`.
    - Если `relative_rank % (2 * stride) == stride`: Процесс выступает **Получателем**. Он принимает данные от `virt_src = relative_rank - stride`.
4.  **Разделение данных:** На каждом этапе отправитель передает часть данных, предназначенную для поддерева получателя.

**Пример схемы:**
```
Шаг 1 (Stride 4): 0 -> 4
Шаг 2 (Stride 2): 0 -> 2, 4 -> 6
Шаг 3 (Stride 1): 0 -> 1, 2 -> 3, 4 -> 5, 6 -> 7
```

---

## 5. Детали реализации
**`ops_mpi.cpp`**: Содержит MPI реализацию.
    - **`ValidationImpl`**: Проверка корректности аргументов (размеры, типы, ранг корня).
    - **`PreProcessingImpl`**: Нормализация ранга root.
    - **`RunImpl`**: Управляет подготовкой буферов и вызывает цикл рассылки.
    - **`PrepareRootData`**: Вспомогательная функция. Выполняет преобразование исходного буфера так, чтобы данные для физического root оказались в начале виртуального буфера. Реализована через `std::copy`.
    - **`ExecScatterCycle`**: Вынесенная в отдельную функцию логика дерева (цикл `for` с битовыми сдвигами). Принимает указатель на активные данные по ссылке и обновляет его при получении новых данных.

**`ops_seq.cpp`**: Реализует последовательную логику копирования.

---

## 6. Экспериментальная установка

**Оборудование и ПО:**
- Компьютер: Apple MacBook Air
- Процессор: Apple Silicon
- Компилятор: Clang
- MPI: OpenMPI 4.1.5
- Тестовый фреймворк: GoogleTest

**Конфигурация тестов:**
- Функциональные тесты: Проверяют корректность работы для разных комбинаций типов данных (INT, FLOAT, DOUBLE), размеров данных (от 1 до 1000 элементов) и рангов корневого процесса (0-3).
- Тесты производительности: Измеряют время выполнения операции `Scatter` для фиксированного объема данных (10 миллионов целых чисел на процесс) при разном количестве процессов (1, 2, 4, 6, 8).

---

## 7. Результаты и обсуждение

### 7.1 Корректность
Все 15 функциональных тестов прошли успешно как для последовательной, так и для MPI-реализации. Это подтверждает корректность работы алгоритма:
- Правильное распределение данных между процессами.
- Корректная обработка различных типов данных (int, float, double).
- Работа с разными рангами корневого процесса.
- Соответствие ожидаемым значениям данных (проверка по формулам генерации).

### 7.2 Производительность
**Условия тестирования:** Тесты производительности запускались на объеме данных 10 миллионов элементов типа `int` на каждый процесс. Таким образом, общий объем данных, обрабатываемых root-процессом, составляет `10M * количество_процессов` элементов.

| Количество процессов | Время выполнения (сек) | Ускорение (относительно 1 процесса) |
|---------------------|------------------------|-------------------------------------|
| SEQ (1 процесс)     | 0.010316               | -                                   |
| MPI (1 процесс)     | 0.013641               | 0.76x (замедление)                  |
| MPI (2 процесса)    | 0.027952               | 0.49x (замедление)                  |
| MPI (4 процесса)    | 0.065218               | 0.21x (замедление)                  |
| MPI (6 процессов)   | 0.176000               | 0.08x (замедление)                  |
| MPI (8 процессов)   | 0.299765               | 0.05x (замедление)                  |

**Анализ результатов:**
1. **SEQ vs MPI (1 процесс):** MPI-версия работает медленнее последовательной из-за накладных расходов на проверки и подготовку данных для алгоритма дерева, который в случае одного процесса избыточен.
2. **Рост времени с увеличением процессов:** Время выполнения увеличивается с ростом количества процессов, что противоречит ожиданиям от параллельного алгоритма. Это объясняется следующими факторами:
   - **Алгоритмическая сложность:** Биномиальное дерево имеет временную сложность O(log₂P), но каждый шаг включает сериализованные операции `MPI_Send/MPI_Recv`.
   - **Накладные расходы коммуникации:** На локальной машине с shared memory накладные расходы на организацию MPI-коммуникаций становятся значимыми по сравнению с фактическим временем передачи данных.
   - **Последовательная подготовка данных:** Функция `PrepareRootData` выполняется только на root, но создает копию всего буфера.

---

## 8. Заключение

В ходе работы была успешно реализована операция `Scatter` с использованием алгоритма биномиального дерева. Реализация удовлетворяет всем поставленным требованиям:
- Поддерживает типы данных `MPI_INT`, `MPI_FLOAT`, `MPI_DOUBLE`.
- Корректно работает с любым корневым процессом.
- Использует только `MPI_Send` и `MPI_Recv`.
- Прошла все функциональные тесты на корректность.

---


## 9. Литература
1. Стандарт MPI.
2. Лекции и практики по параллельному программированию.

## 10. Приложение

```cpp
namespace {
size_t GetTypeByteSize(MPI_Datatype type) {
  MPI_Aint lb = 0;
  MPI_Aint extent = 0;
  MPI_Type_get_extent(type, &lb, &extent);
  return static_cast<size_t>(extent);
}

int MapVirtToReal(int virt_rank, int root, int size) {
  return (virt_rank + root) % size;
}

std::vector<uint8_t> PrepareRootData(const void *src, int size, int root, int count, size_t type_size) {
  size_t chunk_bytes = static_cast<size_t>(count) * type_size;
  size_t total_bytes = static_cast<size_t>(size) * chunk_bytes;

  std::vector<uint8_t> buffer(total_bytes);

  const auto *src_bytes = static_cast<const uint8_t *>(src);
  uint8_t *dst_bytes = buffer.data();

  size_t offset = root * chunk_bytes;
  size_t tail_size = total_bytes - offset;

  std::copy(src_bytes + offset, src_bytes + total_bytes, dst_bytes);

  std::copy(src_bytes, src_bytes + offset, dst_bytes + tail_size);

  return buffer;
}

void ExecScatterCycle(int size, int root, int rank, int count, MPI_Datatype type, size_t type_size, MPI_Comm comm,
                      const uint8_t *&active_ptr, std::vector<uint8_t> &buffer) {
  int relative_rank = (rank - root + size) % size;

  int start_stride = 1;
  while (start_stride < size) {
    start_stride <<= 1;
  }
  start_stride >>= 1;

  for (int stride = start_stride; stride > 0; stride >>= 1) {
    if (relative_rank % stride != 0) {
      continue;
    }

    bool is_sender = (relative_rank % (stride << 1) == 0);

    if (is_sender) {
      int virt_dest = relative_rank + stride;

      if (virt_dest < size) {
        int limit = std::min(virt_dest + stride, size);
        int send_amt = (limit - virt_dest) * count;

        size_t byte_shift = static_cast<size_t>(virt_dest - relative_rank) * count * type_size;
        int real_dest = MapVirtToReal(virt_dest, root, size);

        MPI_Send(active_ptr + byte_shift, send_amt, type, real_dest, 0, comm);
      }
    } else {
      int virt_src = relative_rank - stride;
      int real_src = MapVirtToReal(virt_src, root, size);

      int limit = std::min(relative_rank + stride, size);
      int recv_amt = (limit - relative_rank) * count;

      size_t required_bytes = static_cast<size_t>(recv_amt) * type_size;
      buffer.resize(required_bytes);

      MPI_Recv(buffer.data(), recv_amt, type, real_src, 0, comm, MPI_STATUS_IGNORE);

      active_ptr = buffer.data();
    }
  }
}

}  // namespace

bool KrymovaKScatterMPI::RunImpl() {
  auto &args = GetInput();

  int rank = 0;
  int size = 0;
  MPI_Comm_rank(args.comm, &rank);
  MPI_Comm_size(args.comm, &size);

  size_t type_size = GetTypeByteSize(args.recv_type);

  std::vector<uint8_t> internal_buf;
  const uint8_t *active_data_ptr = nullptr;

  if (rank == args.root_rank) {
    internal_buf = PrepareRootData(args.src_buffer, size, args.root_rank, args.recv_count, type_size);
    active_data_ptr = internal_buf.data();
  }

  ExecScatterCycle(size, args.root_rank, rank, args.recv_count, args.recv_type, type_size, args.comm, active_data_ptr,
                   internal_buf);

  if (args.dst_buffer != MPI_IN_PLACE && active_data_ptr != nullptr) {
    size_t bytes_to_copy = args.recv_count * type_size;
    auto *user_dst = static_cast<uint8_t *>(args.dst_buffer);
    std::copy(active_data_ptr, active_data_ptr + bytes_to_copy, user_dst);
  }

  size_t result_bytes = args.recv_count * type_size;
  std::vector<uint8_t> result(result_bytes);

  const void *final_src = (args.dst_buffer != nullptr) ? args.dst_buffer : active_data_ptr;
  if (final_src != nullptr) {
    const auto *ptr = static_cast<const uint8_t *>(final_src);
    std::copy(ptr, ptr + result_bytes, result.begin());
  }

  GetOutput() = std::move(result);
  return true;
}
```