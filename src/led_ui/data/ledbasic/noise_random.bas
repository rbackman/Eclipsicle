setup
  brightness(128)
  clear()
end

loop(time)
  for i = 0 to numled()-1
    r = random() * 255
    g = random() * 255
    b = random() * 255
    setled(i, r, g, b)
  next
  show()
end

