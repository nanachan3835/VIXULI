echo "kiếm tra môi trường python"
# Kiểm tra biến môi trường IDF_PYTHON_ENV_PATH
echo $IDF_PYTHON_ENV_PATH

echo "kiểm tra biến môi trường IDF_PYTHON_ENV_PATH"
if [ -z "$IDF_PYTHON_ENV_PATH" ]; then
    echo "IDF_PYTHON_ENV_PATH is not set. Please set it to the path of your Python environment."
    exit 1
fi

echo "kích hoạt môi trường python của espidf"

source $IDF_PYTHON_ENV_PATH/bin/activate