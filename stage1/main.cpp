#include "sample_double.h"

#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace
{
void Expect(bool condition)
{
    if (!condition)
    {
        throw std::runtime_error("Проверка нешаблонного класса не пройдена.");
    }
}

class SampleAccess : public SampleDouble
{
public:
    using SampleDouble::CheckSquare;
};
}

int main()
{
    try
    {
        SampleAccess sample;
        Expect(sample.IsZeroSize() && sample.GetPointer() == nullptr);
        sample.Resize(2);
        Expect(sample.IsSquare() && sample.GetSize() == 2);
        sample.CheckSquare();
        sample.Resize(2, 3);
        Expect(sample.GetSizeX() == 2 && sample.GetSizeY() == 3 && !sample.IsSquare());
        sample.SetValue(2.0);
        sample(1, 2) = 6.0;
        const SampleDouble& constant = sample;
        Expect(constant(1, 2) == 6.0 && constant.GetPointer()[5] == 6.0);
        sample.GetPointer()[0] = 4.0;
        Expect(sample(0, 0) == 4.0 && sample.GetMax() == 6.0);

        SampleDouble copied(sample);
        copied(0, 0) = 9.0;
        Expect(sample(0, 0) == 4.0 && copied(0, 0) == 9.0);
        SampleDouble assigned;
        Expect(&(assigned = sample) == &assigned && assigned.IsEqualSize(sample));
        assigned += 1.0;
        assigned *= 2.0;
        assigned /= 2.0;
        Expect(assigned(0, 0) == 5.0 && assigned(1, 2) == 7.0);
        assigned += sample;
        Expect(assigned(1, 2) == 13.0);
        assigned -= sample;
        Expect(assigned(1, 2) == 7.0);
        assigned *= sample;
        Expect(assigned(0, 0) == 20.0 && assigned(1, 2) == 42.0);

        std::stringstream stream;
        assigned.Save(stream);
        SampleDouble restored;
        restored.Load(stream);
        Expect(restored.IsEqualSize(assigned) && restored(1, 2) == 42.0);

        bool caught = false;

        try
        {
            restored /= 0.0;
        }
        catch (const std::invalid_argument&)
        {
            caught = true;
        }

        Expect(caught && restored(1, 2) == 42.0);
        std::cout << "Первый этап (SampleDouble): все проверки пройдены.\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
