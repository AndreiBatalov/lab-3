#include "sample.h"

#include <complex>
#include <exception>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace
{
void Expect(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

template <class Exception, class Function>
void ExpectException(Function function)
{
    try
    {
        function();
    }
    catch (const Exception&)
    {
        return;
    }

    throw std::runtime_error("Ожидаемое исключение не возникло.");
}

class TestRunner
{
public:
    template <class Function>
    void Run(const std::string& name, Function function)
    {
        try
        {
            function();
            ++m_passed;
            std::cout << "[OK] " << name << '\n';
        }
        catch (const std::exception& error)
        {
            ++m_failed;
            std::cout << "[FAIL] " << name << ": " << error.what() << '\n';
        }
    }

    int Finish() const
    {
        std::cout << "\nУспешно: " << m_passed << ", ошибок: " << m_failed << '\n';
        return m_failed == 0 ? 0 : 1;
    }

private:
    int m_passed = 0;
    int m_failed = 0;
};

template <class PAR>
class SampleAccess : public Sample<PAR>
{
public:
    using Sample<PAR>::CheckSquare;
};

class TrackedSample : public Sample<double>
{
public:
    explicit TrackedSample(bool& destroyed)
        : m_destroyed(destroyed)
    {
    }

    ~TrackedSample() override
    {
        m_destroyed = true;
    }

private:
    bool& m_destroyed;
};

template <class PAR>
void TestSample(TestRunner& runner, const std::string& typeName)
{
    const std::string prefix = typeName + ": ";

    runner.Run(prefix + "конструктор и пустая выборка", []
    {
        const Sample<PAR> sample;
        Expect(sample.GetSize() == 0 && sample.GetSizeX() == 0 && sample.GetSizeY() == 0,
               "Неверные начальные размеры.");
        Expect(sample.IsSquare() && sample.IsZeroSize(), "Неверные свойства пустой выборки.");
        Expect(sample.GetPointer() == nullptr, "У пустой выборки должен быть nullptr.");
        ExpectException<std::runtime_error>([&sample] { sample.GetMax(); });
    });

    runner.Run(prefix + "Resize и получение размеров", []
    {
        Sample<PAR> sample;
        sample.Resize(2);
        Expect(sample.GetSize() == 2 && sample.GetSizeX() == 2 && sample.GetSizeY() == 2,
               "Неверный размер квадратной выборки.");
        Expect(sample.IsSquare() && !sample.IsZeroSize(), "Неверные свойства квадрата.");
        Expect(sample(0, 0) == PAR{} && sample(1, 1) == PAR{}, "Элементы не инициализированы.");
        sample(0, 0) = PAR{7};
        sample.Resize(2, 3);
        Expect(sample.GetSizeX() == 2 && sample.GetSizeY() == 3 && !sample.IsSquare(),
               "Неверный размер прямоугольной выборки.");
        Expect(sample(0, 0) == PAR{7} && sample(1, 2) == PAR{}, "Неверное сохранение данных.");
        ExpectException<std::invalid_argument>([&sample] { sample.Resize(-1); });
        ExpectException<std::invalid_argument>([&sample] { sample.Resize(-1, 2); });
        ExpectException<std::invalid_argument>([&sample] { sample.Resize(2, -1); });
        Expect(sample.GetSizeX() == 2 && sample.GetSizeY() == 3, "Ошибка изменила размеры.");
        sample.Resize(0, 3);
        Expect(sample.IsZeroSize() && sample.GetPointer() == nullptr, "Неверная нулевая выборка.");
        sample.Resize(0);
        Expect(sample.IsSquare() && sample.IsZeroSize(), "Неверный нулевой квадрат.");
    });

    runner.Run(prefix + "индексирование и GetPointer", []
    {
        Sample<PAR> sample;
        sample.Resize(2, 3);

        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                sample(i, j) = static_cast<PAR>(i * 3 + j + 1);
            }
        }

        const Sample<PAR>& constant = sample;
        Expect(sample.GetPointer()[5] == PAR{6} && constant(1, 2) == PAR{6},
               "Неверное размещение прямоугольных данных.");
        Expect(constant.GetPointer()[3] == PAR{4}, "Неверный const-указатель.");
        sample.GetPointer()[2] = PAR{9};
        Expect(sample(0, 2) == PAR{9}, "Указатель не изменяет данные.");
        ExpectException<std::out_of_range>([&sample] { sample(-1, 0); });
        ExpectException<std::out_of_range>([&sample] { sample(0, -1); });
        ExpectException<std::out_of_range>([&sample] { sample(2, 0); });
        ExpectException<std::out_of_range>([&sample] { sample(0, 3); });
        ExpectException<std::out_of_range>([&constant] { constant(-1, 0); });
        ExpectException<std::out_of_range>([&constant] { constant(0, -1); });
        ExpectException<std::out_of_range>([&constant] { constant(2, 0); });
        ExpectException<std::out_of_range>([&constant] { constant(0, 3); });
        sample.Resize(0);
        ExpectException<std::out_of_range>([&sample] { sample(0, 0); });
    });

    runner.Run(prefix + "SetValue и GetMax", []
    {
        Sample<PAR> sample;
        sample.Resize(2, 3);
        sample.SetValue(PAR{-4});

        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                Expect(sample(i, j) == PAR{-4}, "Не все элементы заполнены.");
            }
        }

        Expect(sample.GetMax() == PAR{-4}, "Неверный максимум отрицательных элементов.");
        sample(1, 2) = PAR{8};
        Expect(sample.GetMax() == PAR{8}, "Неверный максимум.");
        sample.Resize(0);
        sample.SetValue(PAR{1});
        ExpectException<std::runtime_error>([&sample] { sample.GetMax(); });
    });

    runner.Run(prefix + "IsEqualSize и CheckSquare", []
    {
        SampleAccess<PAR> first;
        Sample<PAR> second;
        first.Resize(2, 3);
        second.Resize(2, 3);
        Expect(first.IsEqualSize(second), "Одинаковые размеры не распознаны.");
        second.Resize(3, 2);
        Expect(!first.IsEqualSize(second), "Переставленные размеры распознаны как равные.");
        ExpectException<std::runtime_error>([&first] { first.CheckSquare(); });
        first.Resize(2);
        first.CheckSquare();
        first.Resize(0);
        first.CheckSquare();
    });

    runner.Run(prefix + "конструктор копирования и присваивание", []
    {
        Sample<PAR> original;
        original.Resize(2, 3);
        original.SetValue(PAR{5});
        Sample<PAR> copied(original);
        copied(1, 2) = PAR{9};
        Expect(original(1, 2) == PAR{5} && copied(1, 2) == PAR{9}, "Копия разделяет данные.");
        Sample<PAR> assigned;
        assigned.Resize(1);
        Expect(&(assigned = original) == &assigned, "Присваивание не возвращает ссылку.");
        Expect(assigned.IsEqualSize(original) && assigned(1, 2) == PAR{5}, "Неверное присваивание.");
        assigned(0, 0) = PAR{3};
        Expect(original(0, 0) == PAR{5}, "Присвоенный объект разделяет данные.");
        const Sample<PAR>& alias = assigned;
        assigned = alias;
        Expect(assigned(0, 0) == PAR{3}, "Самоприсваивание изменило данные.");
        Sample<PAR> first;
        Sample<PAR> second;
        first = second = original;
        Expect(first(1, 2) == PAR{5} && second(1, 2) == PAR{5}, "Неверное цепное присваивание.");
        assigned = Sample<PAR>{};
        Expect(assigned.IsZeroSize() && assigned.GetPointer() == nullptr, "Пустое присваивание.");
    });

    runner.Run(prefix + "операции с числом", []
    {
        Sample<PAR> sample;
        sample.Resize(2, 3);
        sample.SetValue(PAR{2});
        Expect(&(sample += PAR{1}) == &sample, "+= не возвращает ссылку.");
        Expect(&(sample *= PAR{4}) == &sample, "*= не возвращает ссылку.");
        Expect(&(sample /= PAR{2}) == &sample, "/= не возвращает ссылку.");

        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                Expect(sample(i, j) == PAR{6}, "Неверная арифметика с числом.");
            }
        }

        ExpectException<std::invalid_argument>([&sample] { sample /= PAR{}; });
        Expect(sample(1, 2) == PAR{6}, "Деление на ноль изменило данные.");
        sample.Resize(0);
        sample += PAR{1};
        sample *= PAR{2};
        sample /= PAR{2};
        Expect(sample.IsZeroSize(), "Операция создала элементы пустой выборки.");
    });

    runner.Run(prefix + "операции с двумя выборками", []
    {
        Sample<PAR> left;
        Sample<PAR> right;
        left.Resize(2, 3);
        right.Resize(2, 3);

        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                left(i, j) = static_cast<PAR>(i * 3 + j + 1);
            }
        }

        right.SetValue(PAR{2});
        Expect(&(left += right) == &left, "+= не возвращает ссылку.");
        Expect(left(0, 0) == PAR{3} && left(1, 2) == PAR{8}, "Неверное сложение выборок.");
        Expect(&(left -= right) == &left, "-= не возвращает ссылку.");
        Expect(left(0, 0) == PAR{1} && left(1, 2) == PAR{6}, "Неверное вычитание выборок.");
        Expect(&(left *= right) == &left, "*= не возвращает ссылку.");
        Expect(left(0, 0) == PAR{2} && left(1, 2) == PAR{12}, "Неверное умножение выборок.");
        Expect(right(0, 0) == PAR{2}, "Правый операнд изменён.");
        left += left;
        Expect(left(1, 2) == PAR{24}, "Неверное сложение с собой.");
        left *= left;
        Expect(left(1, 2) == PAR{576}, "Неверное умножение на себя.");
        left -= left;
        Expect(left.GetMax() == PAR{}, "Неверное вычитание из себя.");
    });

    runner.Run(prefix + "несовместимые размеры и пустая арифметика", []
    {
        Sample<PAR> left;
        Sample<PAR> right;
        left.Resize(2, 3);
        right.Resize(3, 2);
        left.SetValue(PAR{7});
        ExpectException<std::invalid_argument>([&left, &right] { left += right; });
        ExpectException<std::invalid_argument>([&left, &right] { left -= right; });
        ExpectException<std::invalid_argument>([&left, &right] { left *= right; });
        Expect(left.GetSizeX() == 2 && left.GetSizeY() == 3 && left(1, 2) == PAR{7},
               "Ошибка совместимости изменила объект.");
        left.Resize(0);
        right.Resize(0);
        left += right;
        left -= right;
        left *= right;
        Expect(left.IsZeroSize(), "Пустая арифметика создала данные.");
    });

    runner.Run(prefix + "Save и Load в потоке", []
    {
        Sample<PAR> original;
        original.Resize(2, 3);
        original.SetValue(PAR{3});
        original(1, 2) = PAR{8};
        std::stringstream stream;
        const auto precision = stream.precision();
        const Sample<PAR>& constant = original;
        Expect(&constant.Save(stream) == &stream, "Save не возвращает поток.");
        Expect(stream.precision() == precision, "Save изменил точность потока.");
        Sample<PAR> restored;
        Expect(&restored.Load(stream) == &stream, "Load не возвращает поток.");
        Expect(restored.IsEqualSize(original), "Размеры не восстановлены.");

        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                Expect(restored(i, j) == original(i, j), "Данные не восстановлены.");
            }
        }

        std::stringstream emptyStream;
        original.Resize(0, 3);
        original.Save(emptyStream);
        restored.Load(emptyStream);
        Expect(restored.GetSizeX() == 0 && restored.GetSizeY() == 3 && restored.IsZeroSize(),
               "Пустая прямоугольная выборка не восстановлена.");
    });

    runner.Run(prefix + "ошибки Save и Load", []
    {
        Sample<PAR> sample;
        sample.Resize(1);
        sample.SetValue(PAR{7});

        for (const std::string text : {"", "bad header", "1", "2 2 1 2", "1 1 text"})
        {
            std::istringstream input(text);
            ExpectException<std::runtime_error>([&sample, &input] { sample.Load(input); });
            Expect(sample.GetSizeX() == 1 && sample.GetSizeY() == 1 && sample(0, 0) == PAR{7},
                   "Неудачная загрузка изменила объект.");
        }

        std::istringstream negativeSize("-1 2");
        ExpectException<std::invalid_argument>([&sample, &negativeSize] { sample.Load(negativeSize); });
        Expect(sample(0, 0) == PAR{7}, "Отрицательный размер изменил объект.");
        std::ostringstream broken;
        broken.setstate(std::ios::badbit);
        ExpectException<std::runtime_error>([&sample, &broken] { sample.Save(broken); });
    });

    runner.Run(prefix + "сохранение и загрузка файла", [typeName]
    {
        const std::string path = "sample_" + typeName + ".txt";
        Sample<PAR> original;
        original.Resize(2, 3);
        original.SetValue(PAR{4});
        original(1, 2) = PAR{9};

        {
            std::ofstream output(path);
            Expect(static_cast<bool>(output), "Не удалось открыть файл для записи.");
            original.Save(output);
        }

        std::ifstream input(path);
        Expect(static_cast<bool>(input), "Не удалось открыть файл для чтения.");
        Sample<PAR> restored;
        restored.Load(input);
        Expect(restored.IsEqualSize(original) && restored(0, 0) == PAR{4}
               && restored(1, 2) == PAR{9}, "Файловый обмен изменил данные.");
    });
}
}

int main()
{
    TestRunner runner;
    TestSample<int>(runner, "int");
    TestSample<double>(runner, "double");

    runner.Run("double: точность вещественных значений при сохранении", []
    {
        Sample<double> original;
        original.Resize(1, 3);
        original(0, 0) = 0.12345678901234567;
        original(0, 1) = std::numeric_limits<double>::min();
        original(0, 2) = std::numeric_limits<double>::max();
        std::stringstream stream;
        original.Save(stream);
        Sample<double> restored;
        restored.Load(stream);

        for (int j = 0; j < 3; ++j)
        {
            Expect(original(0, j) == restored(0, j), "Потеря точности при сохранении.");
        }
    });

    runner.Run("virtual: уничтожение наследника через указатель базового класса", []
    {
        bool destroyed = false;

        {
            const std::unique_ptr<Sample<double>> sample = std::make_unique<TrackedSample>(destroyed);
            sample->Resize(1);
            sample->SetValue(2.0);
            Expect(sample->GetMax() == 2.0, "Ошибка доступа через базовый указатель.");
        }

        Expect(destroyed, "Деструктор наследника не вызван.");
    });

    runner.Run("complex<double>: создание и арифметика шаблонной выборки", []
    {
        using Complex = std::complex<double>;
        Sample<Complex> sample;
        sample.Resize(1);
        sample.SetValue(Complex{1.0, 2.0});
        sample += Complex{1.0, 0.0};
        sample *= Complex{0.0, 1.0};
        sample /= Complex{2.0, 0.0};
        Expect(sample(0, 0) == Complex{-1.0, 1.0}, "Неверная комплексная арифметика.");
    });

    return runner.Finish();
}
