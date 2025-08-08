setup
  brightness(128)
  clear()
end

loop(time)
  for i = 0 to numled()-1
    angle = i * 6.28318 / numled()
    dist = abs(sin(angle + time / 300))
    h = dist * 360
    setled(i, hsv(h, 255, 255))
  next
  show()
end

