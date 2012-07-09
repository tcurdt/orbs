#!/usr/bin/env ruby
require 'matrix'

Axis = Struct.new(:steps, :range)

max_total_size   = 1_000_000_000
max_segment_size = Axis.new(10, 10_000..200_000_000)
sync_freq        = Axis.new(10, 5..10_000)

iterations = 4

def func(x, y, opts={})
  `rm -rf tmp 2>/dev/null`
  `mkdir tmp`
  `cat #{opts[:filename]} | ../build/orbs-pipe -t #{opts[:max_total_size]} -s #{x} -f #{y} tmp`.to_f
end

def benchmarks(xa, ya, opt={})
  Matrix.build(ya.steps,xa.steps) do |y,x|
    printf '.'
    func(
      (xa.range.end - xa.range.begin) / (xa.steps - 1) * x + xa.range.begin,
      (ya.range.end - ya.range.begin) / (ya.steps - 1) * y + ya.range.begin
    )
  end
end

old_b = nil
iterations.times do |i|
  printf "running #{i+1}/#{iterations} "
  b = benchmarks(max_segment_size, sync_freq, { :filename => ARGV[0], :max_total_size => max_total_size })
  if old_b
    b = (old_b + b) / 2
  end
  old_b = b
  puts
end

puts "return { nRows: #{old_b.row_size}, nCols: #{old_b.column_size}, formattedValues: #{old_b.to_a} }"
