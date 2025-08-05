setup
  brightness(128)
  clear()
  speed = 20
end

loop(time)
  for i = 0 to numled()-1
    h = (i * 360 / numled() + time / speed) % 360
    s = 255
    v = 255
    setled(i, hsv(h, s, v))
  next
  show()
end