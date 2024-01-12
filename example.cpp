/* In this program, we define three types `Circle`/`RightTriangle`/`Rectangle`, each of which
implements the functions `get_area()` and `print_info()`. We also define the type `Shape`, which
generalizes between the first three types.

Rather than the traditional approach, where `Shape` is made a an abstract base class with
pure virtual functions `get_area()` and `print_info()` and we have all of `Circle`/`RightTriangle`/
`Rectangle` inherit from `Shape`, we now simply have `Shape` itself inherit from
`TaggedPointer<Circle, RightTriangle, Rectangle>`. `Shape` will implement `get_area()`
and `print_info()`, which both dispatch a call to `get_area()` and `print_info()`, respectively,
to the tagged pointer. This allows us to simulate runtime polymorphism without the storage
overhead of virtual function pointers, which is stored for every pointer to an abstract
base type. */

#include <iostream>
#include <numbers>
#include <cassert>
#include "tagged_pointer.h"

/* `Circle` class, which would traditionally inherit from `Shape` */
struct Circle {
    double radius;

    double get_area() const {
        return std::numbers::pi * radius * radius;
    }

    void print_info() const {
        std::cout << "Circle with radius " << radius << std::endl;
    }
};

/* `RightTriangle` class, which would traditionally inherit from `Shape` */
struct RightTriangle {
    double base, height;

    double get_area() const {
        return base * height / 2.;
    }

    void print_info() const {
        std::cout << "Right triangle with base " << base << " and height " << height << std::endl;
    }
};

/* `Rectangle` class, which would traditionally inherit from `Shape` */
struct Rectangle {
    double width, height;

    double get_area() const {
        return width * height;
    }

    void print_info() const {
        std::cout << "Rectangle with width " << width << " and height " << height << std::endl;
    }
};

/* The `Shape` class, rather than being an abstract class, is now made to inherit from
`TaggedPointer`. */
struct Shape : public TaggedPointer<Circle, RightTriangle, Rectangle> {
    /* `Shape` inherits the `TaggedPointer` constructors */
    using TaggedPointer::TaggedPointer;

    /* Returns the area of this `Shape`. */
    double get_area() const {
        /* Simply dispatch the call to the tagged pointer. This simulates the runtime
        polymorphism we would otherwise have to rely on inheritance to achieve. This
        is the general form of implementing the member functions for base types that
        inherit from `TaggedPointer`. */
        return call([](auto ptr){return ptr->get_area();});
    }

    /* Prints the information for this `Shape`. */
    void print_info() const {
        /* Simply dispatch the call to the tagged pointer. This simulates the runtime
        polymorphism we would otherwise have to rely on inheritance to achieve. */

        /* Apparently even though `print_info()` (and therefore `call`) return `void`,
        you can still use it in a `return` statement. */
        return call([](auto ptr){return ptr->print_info();});
    }
};

int main()
{
    /* No need to handle `Shape` behind a pointer or reference, unlike if `Shape` was an
    abstract base class and `Circle`/`RightTriangle`/`Rectangle` all inherited from it.
    We just declare an object of type `Shape` directly. */
    Shape my_shape;

    /* Assign a pointer to `Circle`/`RightTriangle`/`Rectangle` (which one is chosen is determined
    at runtime) to `my_shape`. */
    std::cout << "Enter a shape (Circle, RightTriangle, or Rectangle): ";

    if (std::string input; std::cin >> input && input == "Circle") {
        my_shape = new Circle{.radius = 1};
    } else if (input == "RightTriangle") {
        my_shape = new RightTriangle{.base = 5, .height = 12};
    } else if (input == "Rectangle") {
        my_shape = new Rectangle{.width = 5, .height = 4};
    } else {
        std::cout << "Did not enter a valid shape" << std::endl;
        return 0;
    }

    /* Call some functions on `my_shape`; the tagged pointer will correct dispatch the function
    calls to the true type of `my_shape`. */
    std::cout << "Created a ";
    my_shape.print_info();
    std::cout << "my_shape.get_area() returned " << my_shape.get_area() << std::endl;

    /* Example: == and != operators for `TaggedPointer` */
    auto my_shape2 = my_shape;
    assert(my_shape == my_shape2);

    my_shape2 = new RightTriangle{.base = 3, .height = 4};
    assert(my_shape != my_shape2);

    return 0;
}