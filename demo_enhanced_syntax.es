#!/Users/immateria/ES-SHELL/es-shell/es

echo "Enhanced Control Structures Demo"
echo "================================"
echo

# Demo 1: Traditional if-else emulation
echo "=== Demo 1: Traditional-style if/elseif/else ==="
x = 5

traditional-if {~ $x 1} {echo "x is one"} \
              elseif {~ $x 2} {echo "x is two"} \
              elseif {~ $x 5} {echo "x is five!"} \
              else {echo "x is something else"}

echo

# Demo 2: Simple if-else for basic cases
echo "=== Demo 2: Simple if-else ==="
age = 25

simple-if {> $age 18} {echo "Adult"} {echo "Minor"}

echo

# Demo 3: Clean list handling
echo "=== Demo 3: Enhanced list utilities ==="
fruits = (apple banana cherry date)

echo "Standard echo with list:"
echo "Fruits: $fruits"

echo "Clean echo with list:"
echo-clean "Fruits:" $fruits

echo "Joined with commas:"
echo "Fruits: <={join ", " $fruits}"

echo

# Demo 4: Case statement
echo "=== Demo 4: Case statement ==="
day = Tuesday

case $day Monday {echo "Start of work week"} \
          Tuesday {echo "Tuesday blues"} \
          Wednesday {echo "Hump day"} \
          Friday {echo "TGIF!"} \
          {echo "Some other day"}

echo

# Demo 5: when/unless helpers  
echo "=== Demo 5: Helper functions ==="
debug = true

when {~ $debug true} {echo "Debug mode is enabled"}
unless {~ $debug false} {echo "Debug is not disabled"}

echo

echo "=== Traditional ES shell syntax still works ==="
if {~ $x 5} {echo "Original ES if syntax works too"}

echo
echo "Demo completed! The syntax is much cleaner now."