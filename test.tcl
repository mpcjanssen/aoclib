load build/libaoclib.so

proc encode {m _} {
    return "[string length $m][string index $m 0]"
}

set re [pcre2 {(.)\1*}]




set s 1321131112
for {set i 0} {$i  < 40} {incr i} {
    set s [$re $s encode]
}

rename $re {}
puts [string length $s]