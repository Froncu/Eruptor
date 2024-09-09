#if not defined UNIQUE_POINTER_HPP
#define UNIQUE_POINTER_HPP

#include <memory>
#include <functional>

namespace eru
{
    template<typename Type>
    using UniquePointer = std::unique_ptr<Type, std::function<void(Type*)>>;
}

#endif