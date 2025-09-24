# Contributing to NeXShell

Thank you for your interest in contributing to NeXShell! This document provides guidelines and information about contributing to this project.

## 🤝 How to Contribute

### Reporting Issues

1. **Check existing issues** first to avoid duplicates
2. **Use issue templates** when creating new issues
3. **Provide detailed information** including:
   - Operating system and version
   - Compiler version (GCC/Clang)
   - NeXShell version
   - Steps to reproduce
   - Expected vs actual behavior

### Suggesting Features

1. **Create a feature request** issue
2. **Explain the use case** and benefits
3. **Provide implementation ideas** if possible
4. **Consider backward compatibility**

### Code Contributions

#### Development Setup

1. **Fork the repository**
   ```bash
   git clone https://github.com/yourusername/NeXShell.git
   cd NeXShell
   ```

2. **Set up development environment**
   ```bash
   # Install dependencies
   sudo apt-get install build-essential cmake pkg-config libcurl4-openssl-dev

   # Install Ollama for AI testing
   curl -fsSL https://ollama.ai/install.sh | sh
   ```

3. **Build and test**
   ```bash
   mkdir build && cd build
   cmake ..
   make
   make test
   ```

#### Code Style Guidelines

**C++ Standards:**
- Use C++20 features appropriately
- Follow RAII principles
- Use smart pointers instead of raw pointers
- Maintain const-correctness
- Use meaningful variable and function names

**Formatting:**
```cpp
// Class naming: PascalCase
class AIAssistant {
public:
    // Method naming: camelCase
    bool autoInitialize(const std::string& modelName);
    
private:
    // Member variables: snake_case with trailing underscore
    std::string current_model_;
    bool ai_enabled_;
};

// Function naming: snake_case for free functions
std::string parse_command_line(const std::string& input);

// Constants: ALL_CAPS
constexpr int MAX_COMMAND_LENGTH = 4096;
```

**Documentation:**
```cpp
/**
 * @brief Brief description of the function
 * @param param_name Description of parameter
 * @return Description of return value
 * @throws std::exception Description of when exceptions are thrown
 */
std::string process_command(const std::string& command);
```

#### Testing

**Test Requirements:**
- All new features must include unit tests
- Tests should cover both success and error cases
- Use meaningful test names
- Test coverage should not decrease

**Writing Tests:**
```cpp
#include <gtest/gtest.h>
#include "shell.h"

TEST(ShellTest, ExecuteSimpleCommand) {
    NeXShell::Shell shell;
    int result = shell.execute_command("echo test");
    EXPECT_EQ(result, 0);
}

TEST(ShellTest, HandleInvalidCommand) {
    NeXShell::Shell shell;
    int result = shell.execute_command("nonexistent_command");
    EXPECT_NE(result, 0);
}
```

#### Pull Request Process

1. **Create a feature branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Make your changes**
   - Follow code style guidelines
   - Add tests for new functionality
   - Update documentation as needed
   - Ensure all tests pass

3. **Commit your changes**
   ```bash
   git add .
   git commit -m "feat: add AI command suggestions feature

   - Implement suggest_commands in AIAssistant
   - Add unit tests for suggestion functionality
   - Update documentation with new command usage
   
   Fixes #123"
   ```

4. **Submit pull request**
   - Use clear, descriptive title
   - Explain what changes were made and why
   - Reference related issues
   - Include screenshots/demos if applicable

#### Commit Message Format

Follow conventional commits format:

```
<type>(<scope>): <description>

<body>

<footer>
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `test`: Adding or updating tests
- `chore`: Maintenance tasks

**Examples:**
```bash
feat(ai): add natural language command processing
fix(parser): handle quoted strings with spaces correctly
docs(readme): update installation instructions
test(shell): add tests for command history functionality
```

## 🏗️ Architecture Guidelines

### Adding New Features

#### New Built-in Commands
```cpp
// 1. Add to BuiltinCommands class
class BuiltinCommands {
    // Add your command handler
    std::string handle_your_command(const std::vector<std::string>& args);
};

// 2. Register in command map
builtin_commands_["yourcommand"] = [this](const auto& args) {
    return handle_your_command(args);
};

// 3. Add help text
help_texts_["yourcommand"] = "Description of your command";
```

#### New AI Features
```cpp
// 1. Add to AIAssistant class
class AIAssistant {
    // Add your AI feature
    std::string your_ai_feature(const std::string& input);
};

// 2. Update system prompt if needed
std::string build_system_prompt() {
    // Include context for your feature
}
```

### Performance Guidelines

- **Minimize memory allocations** in hot paths
- **Use move semantics** when appropriate
- **Avoid copying large objects**
- **Profile performance-critical code**
- **Consider async operations** for I/O

### Error Handling

```cpp
// Use exceptions for exceptional conditions
if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filename);
}

// Use return codes for expected failures
enum class CommandResult {
    Success,
    InvalidArguments,
    ExecutionFailed
};

CommandResult execute_builtin(const std::string& command);
```

## 🧪 Testing Strategy

### Test Categories

1. **Unit Tests** - Test individual components
2. **Integration Tests** - Test component interactions
3. **AI Tests** - Test AI functionality with mock responses
4. **Performance Tests** - Benchmark critical operations

### Testing AI Components

```cpp
// Mock Ollama responses for consistent testing
class MockOllamaConnector : public OllamaConnector {
public:
    std::string query_model(const std::string& prompt, 
                          const std::string& model) override {
        // Return predetermined responses for testing
        return mock_responses_[prompt];
    }
};
```

## 📋 Release Process

### Version Numbering

Follow Semantic Versioning (SemVer):
- **MAJOR**: Incompatible API changes
- **MINOR**: New functionality (backward compatible)
- **PATCH**: Bug fixes (backward compatible)

### Release Checklist

- [ ] All tests pass
- [ ] Documentation updated
- [ ] Changelog updated
- [ ] Version number bumped
- [ ] Performance benchmarks run
- [ ] Security review completed

## 💡 Ideas for Contributions

### Easy Contributions
- Improve error messages
- Add more built-in commands
- Enhance documentation
- Write more tests
- Fix typos and formatting

### Medium Contributions
- Add new AI features
- Improve command parsing
- Enhance shell history
- Add configuration options

### Advanced Contributions
- Performance optimizations
- Advanced AI integrations
- Plugin system
- Cross-platform support

## 🔒 Security Considerations

### Security Review Process

- All AI-related features require security review
- Command validation must be comprehensive
- No user input should be executed without validation
- External dependencies must be vetted

### Reporting Security Issues

**Do not open public issues for security vulnerabilities.**

Instead, email security concerns to: security@nexshell.project

## 📞 Getting Help

- **GitHub Discussions**: For general questions and ideas
- **GitHub Issues**: For bug reports and feature requests
- **Code Review**: All contributions get thorough review
- **Mentoring**: Experienced contributors help newcomers

## 🙏 Recognition

Contributors are recognized in:
- README.md contributors section
- Release notes
- Project documentation
- Special contributor badges

---

Thank you for helping make NeXShell better! 🚀
