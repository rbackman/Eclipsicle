setup
  brightness(128)
  clear()
  speed = 200
end

loop(time)
  for i = 0 to numled()-1
    n = sin(i * 0.3 + time / speed)
    v = (n + 1) * 127
    setled(i, v, v, v)
  next
  show()
end

