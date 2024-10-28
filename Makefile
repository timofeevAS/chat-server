# Название выходного исполняемого файла
TARGET = chat

# Компилятор и флаги компиляции
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g

# Список исходных файлов
SRCS = *.cpp

# Список объектных файлов (замена .cpp на .o)
OBJS = $(SRCS:.cpp=.o)

# Команда "all" будет собирать исполняемый файл
all: $(TARGET)

# Правило сборки исполняемого файла
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Правило компиляции объектных файлов
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Очистка сборочных файлов
clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean
