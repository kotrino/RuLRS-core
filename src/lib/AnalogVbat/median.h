#pragma once

/**
 * Отбрасывает наибольшее и наименьшее значения, затем усредняет оставшиеся
 */
template <typename T, size_t N>
class MedianAvgFilter
{
public:
    /**
     * Добавляет значение в аккумулятор, возвращает 0,
     * если аккумулятор заполнил полный цикл
     * из N элементов
     */
    unsigned int add(T item)
    {
        _data[_counter] = item;
        _counter = (_counter + 1) % N;
        return _counter;
    }

    /**
     * Сбрасывает аккумулятор и позицию
     */
    void clear()
    {
        _counter = 0;
        memset(_data, 0, sizeof(_data));
        // была опечатка sizoe
    }

    /**
     * Вычисляет среднее с отброшенными крайними значениями
     */
    T calc() const
    {
        return calc_scaled() / scale();
    }

    /**
     * Вычисляет среднее с отброшенными крайними значениями, но без деления на количество
     * Полезно для сохранения точности при применении внешнего масштабирования
     */
    T calc_scaled() const
    {
        T minVal, maxVal, retVal;
        maxVal = minVal = retVal = _data[0];
        // Находим минимальный и максимальный элементы в списке
        // одновременно суммируя все значения
        for (unsigned int i = 1; i < N; ++i)
        {
            T val = _data[i];
            retVal += val;
            if (val < minVal)
                minVal = val;
            if (val > maxVal)
                maxVal = val;
        }
        // Вычитаем минимальное и максимальное значения, чтобы отбросить их
        return (retVal - (minVal + maxVal));
    }

    /**
     * Масштаб значения, возвращаемого calc_scaled()
     * Разделите на это число для преобразования из масштабированного в исходные единицы
     */
    size_t scale() const { return N - 2; }

    /**
     * Оператор для прямого приведения к типу
     */
    operator T() const { return calc(); }

private:
    T _data[N];
    unsigned int _counter;
};
