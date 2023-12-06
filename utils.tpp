template <typename T>
bool isInArray(const T arr[], int size, const T &value)
{
    for (int i = 0; i < size; ++i)
    {
        if (arr[i] == value)
        {
            return true; // Value found in the array
        }
    }
    return false; // Value not found in the array
}

template <typename T>
bool isInVector(const std::vector<T> &vec, const T &value)
{
    for (const auto &element : vec)
    {
        if (element == value)
        {
            return true; // Value found in the vector
        }
    }
    return false; // Value not found in the vector
}
