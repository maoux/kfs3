<h1>KFS2</h1>
<p>Kernel from sratch project: my final 42 branch of project</p>
<p>KFS2 is the second out of 10 project which leads you to create your very own kernel step by step</p>
<p>Write a kernel from scratch for x86 32bits arch</p>

<ul>
  <li><p>Write a GDT with atleast 6 segments</p></li>
  <li><ul>
    <li><p>kernel code</p></li>
    <li><p>kernel data</p></li>
    <li><p>kernel stack</p></li>
    <li><p>user code</p></li>
    <li><p>user data</p></li>
    <li><p>user stack</p></li>
  </ul></li>
  <li><p>Declare it to the bios at 0x0000800</p></li>
  <li><p>Write a tool to print the stack</p></li>
  <li><p>Write a very basic shell for future debugging</p></li>
</ul>

Host - Linux xubuntu - 64 bit

<h2>Required packages and tools</h2>
<br />
<p>QEMU emulator version 2.11.1</p>
<p>gcc (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0</p>
<p>GNU assembler (GNU Binutils for Ubuntu) 2.30</p>
<p>NASM version 2.13.02</p>
<p>GNU ld (GNU Binutils for Ubuntu) 2.30</p>
<p>GNU Make 4.1</p>
<p>losetup from util-linux 2.31.1</p>
<p>grub (grub-install (GRUB) 2.02-2ubuntu8.17)</p>
<p>mke2fs</p>
<p>fdisk from util-linux 2.31.1</p>

<h2>How to run the project</h2>
<p>From project's root dir, type <code>make</code> then <code>make install</code></p>
<p>installation require super user rights</p>

<h2>Shortcuts</h2>
<ul>
<li><p><code>f2</code> - previous screen</p></li>
<li><p><code>f3</code> - next screen</p></li>
<li><p><code>f5</code> - scroll up</p></li>
<li><p><code>f6</code> - scroll down</p></li>
<li><p><code>print-stack</code> - print current stack</p></li>
<li><p><code>bt</code> - print current stack</p></li>
<li><p><code>shutdown</code> - quit qemu</p></li>
<li><p><code>halt</code> - quit qemu</p></li>
</ul>
