load build/libaoclib.so

proc encode {m c} {
    return "[string length $m]$c"
}

set re [pcre2 {(.)\1*}]




set s 1321131112
for {set i 0} {$i  < 50} {incr i} {
    set s [join [$re $s encode] ""]
}

rename $re {}
puts [string length $s]