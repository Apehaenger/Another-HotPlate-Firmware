# Internal C/C++ remarks

## When to pass by value, by pointer or by reference

http://www.cplusplus.com/articles/z6vU7k9E/ says:

1. Pass by value when the function does not want to modify the parameter and the value is easy to copy (ints, doubles, char, bool, etc... simple types. std::string, std::vector, and all other STL containers are NOT simple types.)

2. Pass by const pointer when the value is expensive to copy AND the function does not want to modify the value pointed to AND NULL is a valid, expected value that the function handles.

3. Pass by non-const pointer when the value is expensive to copy AND the function wants to modify the value pointed to AND NULL is a valid, expected value that the function handles.

4. Pass by const reference when the value is expensive to copy AND the function does not want to modify the value referred to AND NULL would not be a valid value if a pointer was used instead.

5. Pass by non-cont reference when the value is expensive to copy AND the function wants to modify the value referred to AND NULL would not be a valid value if a pointer was used instead.

6. When writing template functions, there isn't a clear-cut answer because there are a few tradeoffs to consider that are beyond the scope of this discussion, but suffice it to say that most template functions take their parameters by value or (const) reference, however because iterator syntax is similar to that of pointers (asterisk to "dereference"), any template function that expects iterators as arguments will also by default accept pointers as well (and not check for NULL since the NULL iterator concept has a different syntax).

Whereas https://www.geeksforgeeks.org/passing-by-pointer-vs-passing-by-reference-in-c/ states:

- References are usually preferred over pointers whenever we don’t need “reseating”.
- Overall, Use references when you can, and pointers when you have to. But if we want to write C code that compiles with both C and a C++ compiler, you’ll have to restrict yourself to using pointers.

Also: Allen Holub's "Enough Rope to Shoot Yourself in the Foot" lists the following 2 rules:
```
120. Reference arguments should always be `const`
121. Never use references as outputs, use pointers
```
