CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -I.
TARGET   := simulate

SOURCES  := main.cpp cache.cpp
OBJECTS  := $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Build concluído: ./$(TARGET)"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "Arquivos removidos."

test: $(TARGET)
	@echo ""
	@echo "=== Teste 1: 256 sets, bloco 4B, direto mapeado, Random ==="

	@if [ -f tests/bin_100.bin ]; then \
		./$(TARGET) 256 4 1 R 1 tests/bin_100.bin; \
	else \
		echo "  [AVISO] tests/bin_100.bin não encontrado"; \
	fi

	@echo ""
	@echo "=== Teste 2: 256 sets, bloco 4B, direto mapeado, FIFO ==="

	@if [ -f tests/bin_100.bin ]; then \
		./$(TARGET) 256 4 1 F 1 tests/bin_100.bin; \
	else \
		echo "  [AVISO] tests/bin_100.bin não encontrado"; \
	fi

	@echo ""
	@echo "=== Teste 3: 256 sets, bloco 4B, direto mapeado, LRU ==="

	@if [ -f tests/bin_100.bin ]; then \
		./$(TARGET) 256 4 1 L 1 tests/bin_100.bin; \
	else \
		echo "  [AVISO] tests/bin_100.bin não encontrado"; \
	fi

help:
	@echo ""
	@echo "Alvos:"
	@echo "  all     Compila o simulador"
	@echo "  clean   Remove arquivos compilados"
	@echo "  test    Executa testes básicos"
	@echo "  help    Exibe esta mensagem"

.PHONY: all clean test help