/* prim-help.c -- enhanced help system primitives ($Revision: 1.0 $) */

#include "es.h"
#include "prim.h"

/*
 * Help content structure
 */

typedef struct {
    const char *topic;
    const char *short_desc;
    const char *long_desc;
    const char *examples;
} HelpTopic;

static const HelpTopic help_topics[] = {
    {
        "arithmetic",
        "Arithmetic operations and expressions",
        "ES Shell supports arithmetic with the new ${...} syntax:\n"
        "\n"
        "Basic operators:\n"
        "  plus, minus, times, div, mod - word-based operators\n"
        "  >, <, >=, <=, ==, != - comparison operators\n"
        "\n"
        "Operator precedence (highest to lowest):\n"
        "  times, div, mod\n"
        "  plus, minus\n"
        "  comparisons",
        "  # Basic arithmetic\n"
        "  echo ${5 plus 3}        # → 8\n"
        "  echo ${10 minus 4}      # → 6\n"
        "  echo ${6 times 7}       # → 42\n"
        "  echo ${15 div 3}        # → 5\n"
        "  \n"
        "  # With variables\n"
        "  x = 5\n"
        "  echo ${x times 2}       # → 10\n"
        "  \n"
        "  # Comparisons\n"
        "  if {${x} > 3} {echo \"x is greater than 3\"}\n"
        "  \n"
        "  # Complex expressions (proper precedence)\n"
        "  echo ${2 plus 3 times 4}  # → 14 (not 20)"
    },
    {
        "redirection",
        "New arrow-based redirection operators",
        "ES Shell uses arrow operators for redirection:\n"
        "\n"
        "Input/Output:\n"
        "  <-      input redirection (replaces <)\n"
        "  ->      output redirection (replaces >)\n"
        "  ->>     append redirection (replaces >>)\n"
        "  \n"
        "Advanced:\n"
        "  <->     read-write redirection\n"
        "  <->>    open-append redirection\n"
        "  ->-<    open-create redirection\n"
        "  <--<    heredoc syntax (replaces <<)",
        "  # Basic redirection\n"
        "  echo \"hello\" -> output.txt\n"
        "  cat <- input.txt\n"
        "  echo \"line\" ->> log.txt\n"
        "  \n"
        "  # With file descriptors\n"
        "  echo \"error\" ->[2] error.log\n"
        "  \n"
        "  # Heredoc\n"
        "  cat <--< EOF\n"
        "  This is heredoc content\n"
        "  EOF"
    },
    {
        "variables",
        "Variable assignment and usage",
        "ES Shell variables are functional and immutable:\n"
        "\n"
        "Assignment:\n"
        "  var = value          # simple assignment\n"
        "  list = (a b c)       # list assignment\n"
        "  \n"
        "Access:\n"
        "  $var                 # variable value\n"
        "  $#var                # count elements\n"
        "  $^var                # flatten list\n"
        "  \n"
        "Expressions:\n"
        "  result = ${var plus 5}   # use variables in expressions",
        "  # Variable basics\n"
        "  name = Alice\n"
        "  echo \"Hello $name\"      # → Hello Alice\n"
        "  \n"
        "  # Lists\n"
        "  colors = (red blue green)\n"
        "  echo $#colors             # → 3\n"
        "  echo $colors              # → red blue green\n"
        "  \n"
        "  # In expressions\n"
        "  count = 5\n"
        "  total = ${count times 10} # → 50"
    },
    {
        "functions",
        "Function definition and usage",
        "ES Shell functions are first-class values:\n"
        "\n"
        "Definition:\n"
        "  fn name params { body }     # define function\n"
        "  fn name { body }            # no parameters\n"
        "  \n"
        "Lambda functions:\n"
        "  @ params { body }           # anonymous function\n"
        "  \n"
        "Calling:\n"
        "  name arg1 arg2              # call function",
        "  # Simple function\n"
        "  fn greet name {\n"
        "      echo \"Hello $name!\"\n"
        "  }\n"
        "  greet Alice               # → Hello Alice!\n"
        "  \n"
        "  # Function with multiple params\n"
        "  fn add x y {\n"
        "      echo ${x plus y}\n"
        "  }\n"
        "  add 5 3                   # → 8\n"
        "  \n"
        "  # Lambda function\n"
        "  square = @ x { echo ${x times x} }\n"
        "  $square 4                 # → 16"
    },
    {
        "primitives",
        "Built-in primitive operations",
        "ES Shell provides many built-in primitives:\n"
        "\n"
        "Access with $& prefix:\n"
        "  %primitive args           # call primitive directly\n"
        "  $&primitive args          # alternative syntax\n"
        "  \n"
        "Common primitives:\n"
        "  %addition, %subtraction, %multiplication, %division\n"
        "  %count, %flatten, %split\n"
        "  %greater, %less, %equal\n"
        "  \n"
        "List primitives with: echo $&primitives",
        "  # Math primitives\n"
        "  echo %addition 5 3        # → 8\n"
        "  echo %multiplication 6 7  # → 42\n"
        "  \n"
        "  # List operations\n"
        "  echo %count (a b c)       # → 3\n"
        "  echo %split ':' a:b:c     # → (a b c)\n"
        "  \n"
        "  # See all primitives\n"
        "  echo $&primitives | head -5"
    },
    {
        "syntax",
        "Basic ES Shell syntax overview",
        "ES Shell syntax fundamentals:\n"
        "\n"
        "Commands:\n"
        "  command args              # simple command\n"
        "  {command args}            # grouped command\n"
        "  command1; command2        # sequence\n"
        "  \n"
        "Control flow:\n"
        "  if {condition} {then} {else}\n"
        "  for (var = list) { body }\n"
        "  \n"
        "Expressions:\n"
        "  ${expression}             # evaluate expression\n"
        "  `{command}                # command substitution",
        "  # Basic commands\n"
        "  echo hello world\n"
        "  date\n"
        "  ls -la\n"
        "  \n"
        "  # Control flow\n"
        "  if {~ $USER root} {\n"
        "      echo \"Running as root\"\n"
        "  } {\n"
        "      echo \"Regular user\"\n"
        "  }\n"
        "  \n"
        "  # Expressions\n"
        "  current_time = `date\n"
        "  result = ${5 times 6}"
    }
};

static const int num_topics = sizeof(help_topics) / sizeof(help_topics[0]);

/*
 * Help primitives
 */

PRIM(help) {
    char *topic = NULL;
    const HelpTopic *found = NULL;
    int i;
    
    // Get topic if provided
    if (list != NULL) {
        topic = getstr(list->term);
    }
    
    if (topic == NULL) {
        // General help overview
        print("ES Shell Help System\n");
        print("===================\n\n");
        print("ES Shell is a functional shell with modern syntax enhancements.\n\n");
        print("Available help topics:\n");
        
        for (i = 0; i < num_topics; i++) {
            print("  help %-12s - %s\n", 
                  help_topics[i].topic, 
                  help_topics[i].short_desc);
        }
        
        print("\nGeneral commands:\n");
        print("  help <topic>        - Get help on specific topic\n");
        print("  discover            - Discover available commands\n");
        print("  echo $&primitives   - List all primitive operations\n");
        print("\nExamples:\n");
        print("  help arithmetic     - Learn about ${...} expressions\n");
        print("  help redirection    - Learn about -> and <- operators\n");
        print("  help variables      - Learn about variables and lists\n");
        
        return ltrue;
    }
    
    // Find specific topic
    for (i = 0; i < num_topics; i++) {
        if (streq(topic, help_topics[i].topic)) {
            found = &help_topics[i];
            break;
        }
    }
    
    if (found == NULL) {
        print("Help topic '%s' not found.\n\n", topic);
        print("Available topics: ");
        for (i = 0; i < num_topics; i++) {
            print("%s%s", help_topics[i].topic, 
                  (i < num_topics - 1) ? ", " : "\n");
        }
        print("\nTry: help (without arguments) for overview\n");
        return lfalse;
    }
    
    // Display topic help
    print("%s\n", found->short_desc);
    print("%s\n", str("%-*s", (int)strlen(found->short_desc), 
                      "========================================"));
    print("\n%s\n", found->long_desc);
    
    if (found->examples && *found->examples) {
        print("\nExamples:\n");
        print("%s\n", found->examples);
    }
    
    return ltrue;
}

PRIM(discover) {
    char *category = NULL;
    
    // Get category if provided
    if (list != NULL) {
        category = getstr(list->term);
    }
    
    if (category == NULL) {
        // General discovery
        print("ES Shell Discovery\n");
        print("==================\n\n");
        print("Discover ES Shell capabilities:\n\n");
        print("discover commands    - Common commands and usage\n");
        print("discover primitives  - Built-in operations\n");  
        print("discover syntax      - Syntax examples\n");
        print("discover features    - New Phase 1 features\n");
        print("\nOr get help on specific topics:\n");
        print("help arithmetic      - ${...} expressions\n");
        print("help redirection     - -> and <- operators\n");
        print("help variables       - Variable operations\n");
        print("help functions       - Function definition\n");
        
        return ltrue;
    }
    
    if (streq(category, "commands")) {
        print("Common ES Shell Commands\n");
        print("========================\n\n");
        print("Basic commands:\n");
        print("  echo text           - Print text\n");
        print("  date               - Show current date/time\n");
        print("  ls                 - List files\n");
        print("  cat file           - Display file contents\n");
        print("  pwd                - Print working directory\n");
        print("\nES Shell specific:\n");
        print("  help               - This help system\n");
        print("  discover           - Discover features\n");
        print("  echo $&primitives  - List all primitives\n");
        print("\nNew syntax (Phase 1):\n");
        print("  echo ${5 plus 3}   - Expression evaluation\n");
        print("  echo hello -> file - Arrow redirection\n");
        print("  cat <- file        - Arrow input\n");
        
    } else if (streq(category, "primitives")) {
        print("ES Shell Primitives\n");
        print("===================\n\n");
        print("Primitives are built-in operations accessed with %:\n\n");
        print("Arithmetic:\n");
        print("  %addition x y      - Add numbers\n");
        print("  %subtraction x y   - Subtract numbers\n");
        print("  %multiplication x y - Multiply numbers\n");
        print("  %division x y      - Divide numbers\n");
        print("\nComparison:\n");
        print("  %greater x y       - Test if x > y\n");
        print("  %less x y          - Test if x < y\n");
        print("  %equal x y         - Test if x == y\n");
        print("\nLists:\n");
        print("  %count list        - Count list elements\n");
        print("  %split delim str   - Split string by delimiter\n");
        print("  %flatten sep list  - Join list with separator\n");
        print("\nSee all: echo $&primitives\n");
        
    } else if (streq(category, "syntax")) {
        print("ES Shell Syntax Examples\n");
        print("========================\n\n");
        print("Variables:\n");
        print("  name = Alice\n");
        print("  echo $name                    # → Alice\n\n");
        print("Lists:\n");  
        print("  items = (a b c)\n");
        print("  echo $#items                  # → 3\n\n");
        print("Functions:\n");
        print("  fn greet x { echo Hello $x }\n");
        print("  greet World                   # → Hello World\n\n");
        print("Control flow:\n");
        print("  if {${x} > 5} {echo big} {echo small}\n\n");
        print("New expressions (Phase 1):\n");
        print("  result = ${5 times 6}         # → 30\n");
        print("  echo ${x plus y}              # Variables in expressions\n\n");
        print("New redirection (Phase 1):\n");
        print("  echo data -> file.txt         # Output to file\n");
        print("  cat <- input.txt              # Input from file\n");
        
    } else if (streq(category, "features")) {
        print("ES Shell Phase 1 Features\n");
        print("=========================\n\n");
        print("New in Phase 1:\n\n");
        print("🎯 Arrow Redirection:\n");
        print("  ->    output (replaces >)\n");
        print("  <-    input (replaces <)\n");
        print("  ->>   append (replaces >>)\n");
        print("  <->   read-write\n");
        print("  <--<  heredoc (replaces <<)\n\n");
        print("🎯 Expression Evaluation:\n");
        print("  ${5 plus 3}        # → 8\n");
        print("  ${var times 2}     # Variables in expressions\n");
        print("  ${x > y}           # Comparisons\n\n");
        print("🎯 Comparison Operators:\n");
        print("  if {${x} >= 18} {echo adult}\n");
        print("  All operators: > < >= <= == !=\n\n");
        print("🎯 Backward Compatibility:\n");
        print("  Old syntax like 'cmd > file' becomes literal text\n");
        print("  All existing ES Shell features still work\n");
        
    } else {
        print("Unknown category: %s\n\n", category);
        print("Available categories:\n");
        print("  commands, primitives, syntax, features\n");
        print("\nTry: discover (without arguments) for overview\n");
        return lfalse;
    }
    
    return ltrue;
}

PRIM(examples) {
    print("ES Shell Examples\n");
    print("=================\n\n");
    print("Getting started:\n");
    print("  help                    # Show help system\n");
    print("  discover                # Discover features\n");
    print("  echo $&primitives       # List primitives\n\n");
    print("Variables and lists:\n");
    print("  name = Alice\n");
    print("  echo \"Hello $name\"       # → Hello Alice\n");
    print("  nums = (1 2 3)\n");
    print("  echo $#nums             # → 3\n\n");
    print("New expression syntax:\n");
    print("  echo ${5 plus 3}        # → 8\n");
    print("  x = 10\n");
    print("  echo ${x times 2}       # → 20\n");
    print("  echo ${x > 5}           # → 0 (true)\n\n");
    print("New redirection syntax:\n");
    print("  echo \"data\" -> file.txt  # Write to file\n");
    print("  cat <- file.txt         # Read from file\n");
    print("  echo \"more\" ->> file.txt # Append to file\n\n");
    print("Functions:\n");
    print("  fn double x { echo ${x times 2} }\n");
    print("  double 7                # → 14\n\n");
    print("Control flow:\n");
    print("  if {${x} > 15} {\n");
    print("      echo \"x is large\"\n");
    print("  } {\n");
    print("      echo \"x is small\"\n");
    print("  }\n");
    
    return ltrue;
}