setup
  brightness(128)
  clear()
  idx = 0
end

loop(time)
  if time % 1000 < 500
    setled(idx, 255, 0, 0)
  else
    setled(idx, 0, 0, 0)
  end
  show()
end

