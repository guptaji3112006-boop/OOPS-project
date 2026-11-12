const state = { products: [], cart: [], coupon: "" };
const money = (n) => `₹${Number(n).toFixed(2)}`;
function notify(text) {
  const el = document.querySelector("#message");
  el.textContent = text;
  el.classList.add("show");
  setTimeout(() => el.classList.remove("show"), 3000);
}
async function loadProducts() {
  const response = await fetch("/api/products");
  state.products = await response.json();
  renderProducts();
}
function renderProducts() {
  const el = document.querySelector("#catalog");
  if (!state.products.length) {
    el.innerHTML =
      "<p>Inventory is empty. Please ask an admin to add products.</p>";
    return;
  }
  el.innerHTML = state.products
    .map(
      (p) =>
        `<article class="card"><small>${p.type}</small><h3>${p.name}</h3><div class="price">${money(p.price)}</div><button onclick="add(${p.id})">Add to cart</button></article>`,
    )
    .join("");
}
function add(id) {
  const item = state.cart.find((x) => x.id === id);
  if (item) item.quantity++;
  else state.cart.push({ id, quantity: 1 });
  renderCart();
}
function change(id, delta) {
  const item = state.cart.find((x) => x.id === id);
  if (!item) return;
  item.quantity += delta;
  if (item.quantity < 1) state.cart = state.cart.filter((x) => x.id !== id);
  renderCart();
}
function renderCart() {
  let subtotal = 0,
    count = 0;
  const el = document.querySelector("#cartItems");
  el.innerHTML =
    state.cart
      .map((item) => {
        const p = state.products.find((x) => x.id === item.id);
        const value = p.price * item.quantity;
        subtotal += value;
        count += item.quantity;
        return `<div class="cart-row"><div><b>${p.name}</b><br>${money(p.price)} × ${item.quantity}</div><div class="qty"><button onclick="change(${p.id},-1)">−</button>${item.quantity}<button onclick="change(${p.id},1)">+</button></div></div>`;
      })
      .join("") || "<p>Your cart is empty.</p>";
  let discount =
    state.coupon === "SAVE10"
      ? subtotal * 0.1
      : state.coupon === "SAVE20" && subtotal >= 1000
        ? subtotal * 0.2
        : state.coupon !== "SAVE10" && subtotal >= 1000
          ? subtotal * 0.15
          : subtotal >= 500
            ? subtotal * 0.1
            : 0;
  document.querySelector("#cartCount").textContent =
    `${count} item${count === 1 ? "" : "s"}`;
  document.querySelector("#subtotal").textContent = money(subtotal);
  document.querySelector("#discount").textContent = `-${money(discount)}`;
  document.querySelector("#total").textContent = money(subtotal - discount);
}
document.querySelector("#coupon").oninput = (e) => {
  state.coupon = e.target.value.trim().toUpperCase();
  renderCart();
};
document.querySelector("#refresh").onclick = loadProducts;
   document.querySelector("#adminToggle").onclick = () => {
     const admin = document.querySelector("#admin");
     admin.classList.toggle("hidden");
     if (!admin.classList.contains("hidden")) {
       admin.scrollIntoView({ behavior: "smooth", block: "center" });
     }
   };
document.querySelector("#closeModal").onclick = () =>
  document.querySelector("#modal").classList.add("hidden");
document.querySelector("#checkout").onclick = () => {
  if (!state.cart.length) return notify("Add a product before checkout.");
  document.querySelector("#modal").classList.remove("hidden");
};
document.querySelector("#productForm").onsubmit = async (e) => {
  e.preventDefault();
  const form = Object.fromEntries(new FormData(e.target));
  const response = await fetch("/api/products", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(form),
  });
  const data = await response.json();
  if (!response.ok) return notify(data.error);
  notify("Product added");
  e.target.reset();
  loadProducts();
};
document.querySelector("#checkoutForm").onsubmit = async (e) => {
  e.preventDefault();
  const customer = Object.fromEntries(new FormData(e.target));
  const response = await fetch("/api/checkout", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({
      customer,
      card: customer.card,
      coupon: state.coupon,
      items: state.cart,
    }),
  });
  const data = await response.json();
  if (!response.ok) return notify(data.error);
  document.querySelector("#modal").classList.add("hidden");
  state.cart = [];
  renderCart();
  notify(`Payment successful. ${data.receipt} generated.`);
};
loadProducts();
renderCart();
