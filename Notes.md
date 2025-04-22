# Notes

## Vectors

Possible changes to a vector.

## No changes

```cpp
std::vector<int> a = {1, 2, 3};
std::vector<int> b = {1, 2, 3};
```

Expected output:

```json
{}
```

## Change in value - same size

```cpp
std::vector<int> a = {1, 2, 3};
std::vector<int> b = {1, 2, 4};
```

```json
{
    "Changes": {
        "2": 4
    }
}
```

We say index 2 has changed to value 4.

## Change in size - add element

```cpp
std::vector<int> a = {1, 2, 3};
std::vector<int> b = {1, 2, 3, 20};
```

Expected output:

```json
{
    "size": 4,
    "Changes": {
        "3": 20
    }
}
```

We say the new size and that index 3 has changed to value 20.

## Change in size - remove element

```cpp
std::vector<int> a = {1, 2, 3};
std::vector<int> b = {1, 2};
```

Expected output:

```json
    
    {
        "size": 2,
    }
```

We say the new size and that the last element has been removed.

## Change in size - remove element - different values

```cpp
std::vector<int> a = {1, 2, 3};
std::vector<int> b = {1, 3};
```

```json
{
    "size": 2,
    "Changes": {
        "1": 3
    }
}
```

We say the new size and that index 1 has changed to value 3.