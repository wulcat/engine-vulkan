//
// Created by Akira Mujawar on 29/10/24.
//

#ifndef PHYSICAT_BUFFER_HPP
#define PHYSICAT_BUFFER_HPP

#include "utility"

using namespace std;

namespace physicat {
    template <typename T>
    class DoubleBuffer {

    public:
        DoubleBuffer() : Current{}, Final{} {}

        void Swap() {
            std::swap(Current, Final);
        }

        T& GetCurrent() {
            return Current;
        }

        T& GetFinal() {
            return Final;
        }

    private:
        T Current;
        T Final;
    };
}


#endif //PHYSICAT_BUFFER_HPP