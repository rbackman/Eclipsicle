setup
  brightness(128)
  clear()
  cols = 8
end

loop(time)
  clear()
  rows = numled() / cols
  for y = 0 to rows-1
    for x = 0 to cols-1
      idx = y * cols + x
      if x % 2 == 0 or y % 2 == 0
        setled(idx, 0, 0, 255)
      end
    next
  next
  show()
end

